// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <optional>

#include "Engine.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
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

	void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};

