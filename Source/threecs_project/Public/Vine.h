// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class UCableComponent;
class UPhysicsConstraintComponent;
class UPlaneComponent;
class UStaticMeshComponent;
class USphereComponent;

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Vine.generated.h"

UCLASS()
class THREECS_PROJECT_API AVine : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVine();

	UPROPERTY(EditAnywhere)
	TObjectPtr<UCableComponent> Cable;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> VineTop;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UPhysicsConstraintComponent> PhysicsConstraint;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USphereComponent> SphereCollision;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void OnCharacterOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	bool IsOccupied;
};
