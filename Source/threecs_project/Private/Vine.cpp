// Fill out your copyright notice in the Description page of Project Settings.


#include "Vine.h"
#include "CableComponent.h"
#include "Components/SphereComponent.h"
#include "Character/Logic/Base_MyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

// Sets default values
AVine::AVine()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VineTop = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Vine top"));
	RootComponent = VineTop;

	Cable = CreateDefaultSubobject<UCableComponent>(TEXT("Cable"));
	Cable->SetupAttachment(RootComponent);

	Cable->bAttachEnd = false;
	Cable->bAttachStart = true;

	PhysicsConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("Physics Constraint"));
	PhysicsConstraint->SetupAttachment(RootComponent);
	PhysicsConstraint->ConstraintActor1 = this;
	
	// parent the sphere to the cable and attach it to the cable, specifically to cable end
	// on attach we need to set constraint components 2 to be the character (mesh?) and hand_r
	// the cable needs to set attach end to true and attach end to the hand_rSocket on the character mesh
	// drop out when jumping, should NOT reattach until you re-enter
	// will have to see how it plays with the MOVE input...
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collider"));
	SphereCollision->InitSphereRadius(20);
	SphereCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	SphereCollision->SetupAttachment(Cable, FName{"CableEnd"});
	SphereCollision->bHiddenInGame = false;
}

// Called when the game starts or when spawned
void AVine::BeginPlay()
{
	Super::BeginPlay();

	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AVine::OnCharacterOverlap);
	IsOccupied = false;
	
}

void AVine::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	SphereCollision->OnComponentBeginOverlap.RemoveDynamic(this, &AVine::OnCharacterOverlap);
}

// Called every frame
void AVine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AVine::OnCharacterOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("OVERLAP!!!"));
	if (IsOccupied)
		return;
	ABase_MyCharacter* character = Cast<ABase_MyCharacter>(OtherActor);
	if (!character)
		return;
	if (!character->EnterSwingState())
		return;
	Cable->bAttachEnd = true;
	Cable->SetAttachEndTo(OtherActor, FName{ "Mesh" }, FName{ "hand_rSocket" });
	PhysicsConstraint->ConstraintActor2 = OtherActor;
	IsOccupied = true;
	character->GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Flying;
	character->GetCharacterMovement()->StopMovementImmediately();
	//PhysicsConstraint->SetConstraintReferencePosition()
	character->GetCapsuleComponent()->SetSimulatePhysics(true);
	//character->GetCapsuleComponent()->AddForce({ 12000000.0 , 0, 0 });
}

