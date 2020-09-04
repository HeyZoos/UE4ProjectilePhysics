// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectilePhysicsProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

AProjectilePhysicsProjectile::AProjectilePhysicsProjectile() 
{
	this->PrimaryActorTick.bCanEverTick = true;

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
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	ProjectileMovement->OnProjectileBounce.AddDynamic(this, &AProjectilePhysicsProjectile::OnProjectileBounce);
	ProjectileMovement->InitialSpeed = 90000.f;  // NOTE(Jesse) The slowest bullets can travel 180 meters per second irl.
	ProjectileMovement->MaxSpeed = ProjectileMovement->InitialSpeed;
}

void AProjectilePhysicsProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
}

void AProjectilePhysicsProjectile::Tick(float DeltaSeconds)
{
	if (this->LastLocation.IsZero())
	{
		this->LastLocation = this->GetActorLocation();
	}

	DrawDebugLine(
		this->GetWorld(),
		this->LastLocation,
		this->GetActorLocation(),
		FColor::Cyan,
		true,
		100.0
	);

	this->LastLocation = GetActorLocation();
}

void AProjectilePhysicsProjectile::OnProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if (ImpactVelocity.Size() < 16000) {
		return;
	}

	auto PenetrationResult = this->ComputeWallBangExitLocation(ImpactResult, this->GetActorForwardVector());

	if (PenetrationResult.has_value())
	{
		AProjectilePhysicsProjectile* NewProjectile = this->GetWorld()->SpawnActor<AProjectilePhysicsProjectile>(
			/* Location */ PenetrationResult.value(),
			/* Rotation */ this->GetActorRotation()
		);

		NewProjectile->ProjectileMovement->Velocity *= CalculateVelocityAfterPenetratingObject(
			ImpactResult.Location,
			PenetrationResult.value(),
			ImpactVelocity,
			0.08
		) / NewProjectile->ProjectileMovement->Velocity.Size();

		if (ImpactVelocity.Size() < 5000.0) {
			NewProjectile->Destroy();
		}
 
		this->Destroy();
	}
}

float AProjectilePhysicsProjectile::CalculateVelocityAfterPenetratingObject(
	FVector EntryLocation, 
	FVector ExitLocation, 
	FVector InitialVelocity, 
	float Mass
) {
	auto JoulesPerCm = 30000.0;
	auto InitialProjectileEnergy = 0.5 * Mass * InitialVelocity.SizeSquared();
	auto ProjectileEnergyAfterPenetration = InitialProjectileEnergy - (ExitLocation - EntryLocation).Size() * JoulesPerCm;
	return UKismetMathLibrary::Sqrt(ProjectileEnergyAfterPenetration / 0.5 * Mass);
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
