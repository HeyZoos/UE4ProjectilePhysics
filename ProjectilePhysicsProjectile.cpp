// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectilePhysicsProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

AProjectilePhysicsProjectile::AProjectilePhysicsProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AProjectilePhysicsProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;

	CollisionComp->SetGenerateOverlapEvents(true);
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AProjectilePhysicsProjectile::OnBeginOverlap);
}

void AProjectilePhysicsProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
}

void AProjectilePhysicsProjectile::OnBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
) {
	GEngine->AddOnScreenDebugMessage(-1, 10.0, FColor::Green, TEXT("Hello"));
	auto PenetrationResult = this->ComputeWallBangExitLocation(SweepResult, this->GetActorForwardVector());

	if (PenetrationResult.has_value())
	{
		this->GetWorld()->SpawnActor<AProjectilePhysicsProjectile>(
			/* Location */ PenetrationResult.value(),
			/* Rotation */ this->GetActorRotation()
		);

		this->Destroy();
	}
}

std::optional<FVector> AProjectilePhysicsProjectile::ComputeWallBangExitLocation(
	FHitResult HitResult,
	FVector ImpactVelocity,
	float PenetrationDepth
) {
	ImpactVelocity.Normalize();
	ImpactVelocity *= PenetrationDepth;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = {
		UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic),
		UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic),
		UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn),
		UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_PhysicsBody),
		UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Destructible)
	};

	TArray<FHitResult> OutHits;

	UKismetSystemLibrary::SphereTraceMultiForObjects(
		/* WorldContextObject */ this->GetWorld(),
		/* Start */ ImpactVelocity + HitResult.Location,
		/* End */ HitResult.Location,
		/* Radius */ this->CollisionComp->GetScaledSphereRadius(),
		/* ObjectTypes */ ObjectTypes,
		/* bTraceComplex */ false,
		/* ActorsToIgnore */{},
		/* DrawDebugType */ EDrawDebugTrace::Persistent,
		/* OutHits */ OutHits,
		/* bIgnoreHits */ false
	);

	for (auto& OutHit : OutHits)
	{
		if (OutHit.Actor == HitResult.Actor)
		{
			return std::optional<FVector>(OutHit.Location);
		}
	}

	return std::nullopt;
}
