// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class USphereComponent;
struct FInputActionInstance;
class UCameraComponent;
class ABase_MyCharacter;

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Rope.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FRopeAttachDelegate, ARope* const);
DECLARE_MULTICAST_DELEGATE(FRopeDetachDelegate);

UCLASS()
class THREECS_PROJECT_API ARope : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARope();

	UPROPERTY(EditAnywhere)
	TObjectPtr<USkeletalMeshComponent> Rope;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> RopeTop;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USphereComponent> SphereCollision;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UCameraComponent> RopeCamera;

	static FRopeAttachDelegate RopeAttachEvent;

	static FRopeDetachDelegate RopeDetachEvent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere)
	float CameraOffset;

	UPROPERTY(EditAnywhere)
	float SwingCooldown;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION()
	void OnCharacterOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	bool IsOccupied;

	void OnMovementTriggered(const FInputActionInstance& input);

	void SubscribeToMovement();

	void UnsubcribeToMovement();

	void ResetSwingableState();

	float CharacterHorizontalAngle;

	float DefaultLookPitch;

	void ReleaseRope(const FInputActionInstance& input);

	TObjectPtr<ABase_MyCharacter> AttachedCharacter;

	bool CanSwing;

	void StartJump(const FInputActionInstance& input);

	bool HasJumpInputStarted;
};
