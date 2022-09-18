#pragma once
#include "Engine/DataTable.h"
#include "MovementModel.generated.h"

USTRUCT(BlueprintType)
struct FMovementSettings : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RunSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SprintSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UCurveFloat* RotationRateCurve;
};

USTRUCT(BlueprintType)
struct FMovementSettingsStance : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMovementSettings Standing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMovementSettings Crouching;
};

USTRUCT(BlueprintType)
struct FMovementSettingsState : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMovementSettingsStance Looking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMovementSettingsStance Aiming;
};
