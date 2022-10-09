// Fill out your copyright notice in the Description page of Project Settings.


#include "ThirdPersonCameraComponent.h"

#include "DayOne/Character/BaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

FName UThirdPersonCameraComponent::BoneNameRoot(TEXT("root"));
FName UThirdPersonCameraComponent::BoneNameHead(TEXT("head"));
FName UThirdPersonCameraComponent::SocketNameRightShoulder(TEXT("TP_CameraTrace_R"));
FName UThirdPersonCameraComponent::SocketNameLeftShoulder(TEXT("TP_CameraTrace_L"));

UThirdPersonCameraComponent::UThirdPersonCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bRightShoulder = true;
	TargetCameraFOV = 90.0f;
}

// Called when the game starts
void UThirdPersonCameraComponent::BeginPlay()
{
	Super::BeginPlay();

	// Load camera config from data table.
	LoadCameraModel();
}

void UThirdPersonCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	Super::GetCameraView(DeltaTime, DesiredView);

	check(Character && Character->GetMesh());

	// Update camera settings first, this may change cased by character stance and gait state.
	UpdateCameraSettings(DeltaTime);

	// Step 1: Get character's pivot target
	// This is the final target that the camera try to follow,
	// it's always sync with character's location.
	// But camera may have offset to this target.
	FTransform PivotTarget = GetPivotTarget();

	// Step 2: Calculate Target Camera Rotation.
	// Use the Control Rotation and interpolate for smooth camera rotation.
	FRotator CameraRotation = UGameplayStatics::GetPlayerCameraManager(Character, 0)->GetCameraRotation();
	FRotator ControllerRotation = UGameplayStatics::GetPlayerController(Character, 0)->GetControlRotation();
	FRotator TargetCameraRotation = UKismetMathLibrary::RInterpTo(CameraRotation, ControllerRotation, DeltaTime, CurrentCameraSettings.RotationLagSpeed);

	// Step 3: Calculate the Smoothed Pivot Target (Orange Sphere).
	// Get the 3P Pivot Target (Green Sphere) and interpolate using axis independent lag for maximum control.
	FVector LaggedLocation = CalculateAxisIndependentLag(SmoothedPivotTarget.GetLocation(), PivotTarget.GetLocation(), TargetCameraRotation, CurrentCameraSettings.PivotLagSpeed, DeltaTime);
	SmoothedPivotTarget = UKismetMathLibrary::MakeTransform(LaggedLocation, PivotTarget.Rotator());
	
	// Step 4: Calculate Pivot Location (BlueSphere).
	// Get the Smoothed Pivot Target and apply local offsets for further camera control.
	FVector PivotForwardVector = UKismetMathLibrary::GetForwardVector(SmoothedPivotTarget.Rotator()) * CurrentCameraSettings.PivotOffset.X;
	FVector PivotRightVector = UKismetMathLibrary::GetRightVector(SmoothedPivotTarget.Rotator()) * CurrentCameraSettings.PivotOffset.Y;
	FVector PivotUpVector = UKismetMathLibrary::GetUpVector(SmoothedPivotTarget.Rotator()) * CurrentCameraSettings.PivotOffset.Z;
	FVector PivotLocation = SmoothedPivotTarget.GetLocation() + PivotForwardVector + PivotRightVector + PivotUpVector;
	
	// Step 5: Calculate Target Camera Location.
	// Get the Pivot location and apply camera relative offsets.
	FVector CameraForwardVector = UKismetMathLibrary::GetForwardVector(TargetCameraRotation) * CurrentCameraSettings.CameraOffset.X;
	FVector CameraRightVector = UKismetMathLibrary::GetRightVector(TargetCameraRotation) * CurrentCameraSettings.CameraOffset.Y;
	FVector CameraUpVector = UKismetMathLibrary::GetUpVector(TargetCameraRotation) * CurrentCameraSettings.CameraOffset.Z;
	FVector TargetCameraLocation = PivotLocation + CameraForwardVector + CameraRightVector + CameraUpVector;
	
	// Step 6: Trace for an object between the camera and character to apply a corrective offset.
	// Trace origins are set within the Character BP via the Camera Interface.
	// Functions like the normal spring arm, but can allow for different trace origins regardless of the pivot. 
	FVector TraceOrigin;
	float TraceRadius;
	ETraceTypeQuery TraceChannel;
	TArray<AActor*> ActorsToIgnore;
	FHitResult OutHit;
	GetTraceParams(TraceOrigin, TraceRadius, TraceChannel);
	UKismetSystemLibrary::SphereTraceSingle(Character, TraceOrigin, TargetCameraLocation, TraceRadius, TraceChannel, false, ActorsToIgnore, EDrawDebugTrace::None, OutHit, true);
	if (OutHit.bBlockingHit && !OutHit.bStartPenetrating)
	{
		TargetCameraLocation = (OutHit.Location - OutHit.TraceEnd) + TargetCameraLocation;
	}

	// Step 8: Lerp First Person Override and return target camera parameters.

	// Final result
	DesiredView.Location = TargetCameraLocation;
	DesiredView.Rotation = TargetCameraRotation;
	DesiredView.FOV = TargetCameraFOV;
}

FTransform UThirdPersonCameraComponent::GetPivotTarget() const
{
	check(Character && Character->GetMesh());
	check(Character->GetMesh()->DoesSocketExist(BoneNameRoot));
	check(Character->GetMesh()->DoesSocketExist(BoneNameHead));
	
	const FVector HeadLocation = Character->GetMesh()->GetSocketLocation(BoneNameHead);
	const FVector RootLocation = Character->GetMesh()->GetSocketLocation(BoneNameRoot);

	TArray<FVector> Vectors;
	Vectors.Add(HeadLocation);
	Vectors.Add(RootLocation);
	
	const FVector CenterLocation = UKismetMathLibrary::GetVectorArrayAverage(Vectors);

	return UKismetMathLibrary::MakeTransform(CenterLocation, Character->GetActorRotation(), FVector::One());
}

FVector UThirdPersonCameraComponent::CalculateAxisIndependentLag(FVector CurrentLocation,
	                                                             FVector TargetLocation,
	                                                             FRotator CameraRotation,
	                                                             FVector LagSpeeds,
	                                                             float DeltaTime) const
{
	const FRotator CameraRotationYaw(0,CameraRotation.Yaw, 0.0f);

	// I don't why un-rotate the location to camera space and then rotate them back...
	const FVector UnrotatedCurrentLocation = CameraRotationYaw.UnrotateVector(CurrentLocation);
	const FVector UnrotatedTargetLocation = CameraRotationYaw.UnrotateVector(TargetLocation);

	const float UnrotatedLocationX = UKismetMathLibrary::FInterpTo(UnrotatedCurrentLocation.X,
																	UnrotatedTargetLocation.X,
																	DeltaTime, LagSpeeds.X);
	const float UnrotatedLocationY = UKismetMathLibrary::FInterpTo(UnrotatedCurrentLocation.Y,
																	UnrotatedTargetLocation.Y,
																	DeltaTime, LagSpeeds.Y);
	const float UnrotatedLocationZ = UKismetMathLibrary::FInterpTo(UnrotatedCurrentLocation.Z,
																	UnrotatedTargetLocation.Z,
																	DeltaTime, LagSpeeds.Z);

	// Rotate the result location back. restore it.
	return CameraRotationYaw.RotateVector(FVector(UnrotatedLocationX, UnrotatedLocationY, UnrotatedLocationZ));
}

void UThirdPersonCameraComponent::GetTraceParams(FVector& TraceOrigin, float& TraceRadius, ETraceTypeQuery& TraceChannel) const
{
	check(Character && Character->GetMesh());

	// Sphere radius of camera collision trace ray.
	// Hardcode this value is ok.
	TraceRadius = 15.0f;
	TraceChannel = UEngineTypes::ConvertToTraceType(ECC_Camera);
	if (bRightShoulder)
	{
		TraceOrigin = Character->GetMesh()->GetSocketLocation(SocketNameRightShoulder);
	}
	else
	{
		TraceOrigin = Character->GetMesh()->GetSocketLocation(SocketNameLeftShoulder);
	}
}

void UThirdPersonCameraComponent::LoadCameraModel()
{
	check(CameraModel);
	
	const FText StandingString = StaticEnum<EStanceState>()->GetDisplayNameTextByIndex(static_cast<int32>(EStanceState::SS_Standing));
	CameraData.Standing = CameraModel->FindRow<FCameraSettingsGait>(*StandingString.ToString(), "Camera In Standing Context");

	const FText CrouchingString = StaticEnum<EStanceState>()->GetDisplayNameTextByIndex(static_cast<int32>(EStanceState::SS_Crouching));
	CameraData.Crouching = CameraModel->FindRow<FCameraSettingsGait>(*CrouchingString.ToString(), "Camera In Crouching Context");

	check(CameraData.Standing);
	check(CameraData.Crouching);
}

void UThirdPersonCameraComponent::UpdateCameraSettings(float DeltaTime)
{
	check(Character);
	
	if (Character->GetStance() == EStanceState::SS_Standing)
	{
		switch (Character->GetGait())
		{
		case EGaitState::GS_Walking:
			UE_LOG(LogTemp, Warning, TEXT("Switch CameraSettings to Standing Walking"))
			CurrentCameraSettings.CameraOffset = UKismetMathLibrary::VInterpTo(CurrentCameraSettings.CameraOffset,
																				CameraData.Standing->Walking.CameraOffset,
																				DeltaTime, 1.5f);
			CurrentCameraSettings.PivotLagSpeed = UKismetMathLibrary::VInterpTo(CurrentCameraSettings.PivotLagSpeed,
																				 CameraData.Standing->Walking.PivotLagSpeed,
																				 DeltaTime, 1.5f);
			break;
		case EGaitState::GS_Running:
			UE_LOG(LogTemp, Warning, TEXT("Switch CameraSettings to Standing Running"))
			CurrentCameraSettings.CameraOffset = UKismetMathLibrary::VInterpTo(CurrentCameraSettings.CameraOffset,
																				CameraData.Standing->Running.CameraOffset,
																				DeltaTime, 2.0f);
			CurrentCameraSettings.PivotLagSpeed = UKismetMathLibrary::VInterpTo(CurrentCameraSettings.PivotLagSpeed,
																				 CameraData.Standing->Running.PivotLagSpeed,
																				 DeltaTime, 2.0f);
			break;
		case EGaitState::GS_Sprinting:
			UE_LOG(LogTemp, Warning, TEXT("Switch CameraSettings to Standing Sprinting"))
			CurrentCameraSettings.CameraOffset = UKismetMathLibrary::VInterpTo(CurrentCameraSettings.CameraOffset,
																			    CameraData.Standing->Sprinting.CameraOffset,
																			    DeltaTime, 0.35f);
			CurrentCameraSettings.PivotLagSpeed = UKismetMathLibrary::VInterpTo(CurrentCameraSettings.PivotLagSpeed,
																				 CameraData.Standing->Sprinting.PivotLagSpeed,
																				 DeltaTime, 0.35f);
			break;
		default:
			checkNoEntry();
		}
	}
	else if (Character->GetStance() == EStanceState::SS_Crouching)
	{
		switch (Character->GetGait())
		{
		case EGaitState::GS_Walking:
			UE_LOG(LogTemp, Warning, TEXT("Switch CameraSettings to Crouching Walking"))
			CurrentCameraSettings.CameraOffset = UKismetMathLibrary::VInterpTo(CurrentCameraSettings.CameraOffset,
																			    CameraData.Crouching->Walking.CameraOffset,
																			    DeltaTime, 1.5f);
			CurrentCameraSettings.PivotLagSpeed = UKismetMathLibrary::VInterpTo(CurrentCameraSettings.PivotLagSpeed,
			                                                                     CameraData.Crouching->Walking.PivotLagSpeed,
			                                                                     DeltaTime, 1.5f);
			break;
		case EGaitState::GS_Running:
			UE_LOG(LogTemp, Warning, TEXT("Switch CameraSettings to Crouching Running"))
			CurrentCameraSettings.CameraOffset = UKismetMathLibrary::VInterpTo(CurrentCameraSettings.CameraOffset,
																			    CameraData.Crouching->Running.CameraOffset,
																			    DeltaTime, 1.5f);
			CurrentCameraSettings.PivotLagSpeed = UKismetMathLibrary::VInterpTo(CurrentCameraSettings.PivotLagSpeed,
																			     CameraData.Crouching->Running.PivotLagSpeed,
																			     DeltaTime, 1.5f);
			break;
		case EGaitState::GS_Sprinting:
			UE_LOG(LogTemp, Warning, TEXT("Switch CameraSettings to Crouching Sprinting"))
			CurrentCameraSettings.CameraOffset = UKismetMathLibrary::VInterpTo(CurrentCameraSettings.CameraOffset,
																				CameraData.Crouching->Sprinting.CameraOffset,
																				DeltaTime, 1.5f);
			CurrentCameraSettings.PivotLagSpeed = UKismetMathLibrary::VInterpTo(CurrentCameraSettings.PivotLagSpeed,
																				 CameraData.Crouching->Sprinting.PivotLagSpeed,
																				 DeltaTime, 1.5f);
			break;
		default:
			checkNoEntry();
		}
	}
}

