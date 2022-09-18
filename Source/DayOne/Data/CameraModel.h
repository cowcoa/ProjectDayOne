#pragma once
#include "Engine/DataTable.h"
#include "CameraModel.generated.h"

USTRUCT(BlueprintType)
struct FCameraSettings : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FVector CameraOffset;

	UPROPERTY(EditAnywhere)
	FVector PivotOffset;

	UPROPERTY(EditAnywhere)
	FVector PivotLagSpeed;

	UPROPERTY(EditAnywhere)
	float RotationLagSpeed;
};

USTRUCT(BlueprintType)
struct FCameraSettingsGait : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FCameraSettings Walking;

	UPROPERTY(EditAnywhere)
	FCameraSettings Running;

	UPROPERTY(EditAnywhere)
	FCameraSettings Sprinting;
};

struct FCameraData
{
	UPROPERTY(EditAnywhere)
	FCameraSettingsGait* Standing;

	UPROPERTY(EditAnywhere)
	FCameraSettingsGait* Crouching;
};
