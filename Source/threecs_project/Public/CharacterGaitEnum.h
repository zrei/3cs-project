// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedEnum.h"
#include "CharacterGaitEnum.generated.h"

UENUM()
enum class ECharacterGait : uint8
{
	VE_WALK UMETA(DisplayName="Walk"),
	VE_RUN  UMETA(DisplayName="Run")
};