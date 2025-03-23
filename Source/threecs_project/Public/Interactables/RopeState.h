// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RopeState.generated.h"

UENUM(BlueprintType)
enum class ERopeInputState : uint8
{
	SWING UMETA(DisplayName = "Swing"),
	SHIMMY UMETA(DisplayName = "Shimmy")
};
