// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <optional>

#include "DrawDebugHelpers.h"
#include "Engine.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectilePhysicsProjectile.generated.h"

UCLASS(config=Game)
class AProjectilePhysicsProjectile : public AActor
{
	GENERATED_BODY()

	/** Sphere collision component */
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	class USphereComponent* CollisionComp;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class UProjectileMovementComponent* ProjectileMovement;

public:
	AProjectilePhysicsProjectile();

	/** called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Returns CollisionComp subobject **/
	FORCEINLINE class USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ProjectileMovement subobject **/
	FORCEINLINE class UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

	/// <summary>
	/// 
	/// </summary>
	void Tick(float DeltaSeconds) override;

	FVector LastLocation;

	/// <summary>
	/// 
	/// </summary>
	/// <param name="WorldContextObject"></param>
	/// <param name="HitResult"></param>
	/// <param name="ImpactVelocity"></param>
	/// <param name="SphereComponent"></param>
	/// <returns></returns>
	std::optional<FVector> ComputeWallBangExitLocation(
		FHitResult HitResult,
		FVector ImpactVelocity,
		float PenetrationDepth = 500.0f
	);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="ImpactResult"></param>
	/// <param name="ImpactVelocity"></param>
	UFUNCTION()
	void OnProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

	float CalculateVelocityAfterPenetratingObject(
		FVector EntryLocation,
		FVector ExitLocation,
		FVector InitialVelocity,
		float Mass
	);
};

