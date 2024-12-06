// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedEnum.h"
#include "CharacterMovementStateEnum.generated.h"

UENUM()
enum class ECharacterMovementState : uint8
{
	VE_IDLE UMETA(DisplayName="Idle"),
	VE_MOVING UMETA(DisplayName="Moving")
};