#pragma once

UENUM(BlueprintType)
enum class ETurnInPlaceState : uint8
{
	ETIPS_No UMETA(DisplayName = "Not turn"),
	ETIPS_Left UMETA(DisplayName = "Turn Left"),
	ETIPS_Right UMETA(DisplayName = "Turn Right"),
	
	ETIPS_MAX
};
