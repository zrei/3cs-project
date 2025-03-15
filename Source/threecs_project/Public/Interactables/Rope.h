// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

struct FInputActionInstance;
class UCameraComponent;
class ABase_MyCharacter;
class URopeMovementSettings;

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Data/Rope/RopeSettings.h"
#include "Interactables/RopeState.h"
#include "Rope.generated.h"

UCLASS()
class THREECS_PROJECT_API ARope : public AActor
{
	GENERATED_BODY()

#pragma region Initialisation
public:	
	// Sets default values for this actor's properties
	ARope();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(EditAnywhere)
	TObjectPtr<URopeSettings> RopeSettings;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	static constexpr unsigned int TOTAL_BONES = 120;

private:
	inline FString GetBoneName(unsigned int boneNumber)
	{
		FString boneNumberString = FString::FromInt(boneNumber);
		if (boneNumber < 10)
		{
			boneNumberString = FString{"00"} + boneNumberString;
		}
		else if (boneNumber < 100)
		{
			boneNumberString = FString{ "0" } + boneNumberString;
		}
		return RopeSettings->BonePrefix + boneNumberString;
	}

	inline int GetBoneNumber(FString boneName)
	{
		boneName.RemoveFromStart(RopeSettings->BonePrefix);
		return FCString::Atoi(*boneName);
	}

	FName BoneToSimulatePhysics;

#pragma endregion

#pragma region Rope
public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> RopeTop;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USkeletalMeshComponent> Rope;

#pragma endregion

#pragma region Camera
public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCameraComponent> RopeCamera;

private:
	float HorizontalCameraOffset;

	float VerticalCameraOffset;

private:
	void UpdateCameraPositionAndRotation();

	float DefaultLookPitch;
#pragma endregion

#pragma region Collision
private:
	UFUNCTION()
	void OnCharacterOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	inline void ActivateCollision()
	{
		for (const TObjectPtr<USceneComponent>& child : Rope->GetAttachChildren())
		{
			TObjectPtr<USphereComponent> sphere = Cast<USphereComponent>(child);
			if (!sphere)
				continue;
			sphere->OnComponentBeginOverlap.AddDynamic(this, &ARope::OnCharacterOverlap);
		}
	}

	inline void DeactivateCollision()
	{
		for (const TObjectPtr<USceneComponent>& child : Rope->GetAttachChildren())
		{
			TObjectPtr<USphereComponent> sphere = Cast<USphereComponent>(child);
			if (!sphere)
				continue;
			sphere->OnComponentBeginOverlap.RemoveAll(this);
		}
	}
#pragma endregion

#pragma region Attach
private:
	bool TryAttachCharacter(ABase_MyCharacter* character, const FName& attachBone);

	TObjectPtr<ABase_MyCharacter> AttachedCharacter;

	float TargetCharacterAttachHorizontalAngle;

	float CurrCharacterAttachHorizontalAngle;

	unsigned int AttachedBone;

	FName AttachedBoneName;

	FName VisualAttachedBoneName;

	constexpr static float InitialVelocityMultiplier = 5;

#pragma endregion

#pragma region Detach
private:
	bool TryDetachCharacter();

	constexpr static float LaunchVelocityMultiplier = 5;
#pragma endregion

#pragma region Input
private:
	void CheckCharacterInputs(float deltaTime);

	FName BoneToApplyForce;
#pragma endregion

#pragma region Swing
private:
	void OnSwing(FVector2D normalizedMovementInput, float deltaTime) const;
#pragma endregion

#pragma region Shimmy
private:
	void OnShimmy(FVector2D normalizedMovementInput, float deltaTime);

	void AdjustHorizontalPosition(float deltaTime);

protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<URopeMovementSettings> RopeMovementSettings;
#pragma endregion

#pragma region State
private:
	bool IsOccupied;

	bool CanSwing;

	void ResetSwingableState();

	FTimerHandle ResetSwingStateTimer;
#pragma endregion
};
