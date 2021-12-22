// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "RPGProjectile.generated.h"

class USphereComponent;
class UParticleSystemComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UDataTable;

USTRUCT(BlueprintType)
struct FProjectileEffectsData : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
		class UParticleSystem* BaseEffect;

	UPROPERTY(EditAnywhere)
		class UParticleSystem* ImpactEffect;

};

USTRUCT(BlueprintType)
struct FProjectileData : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FString Name;

	UPROPERTY(EditAnywhere)
	class UTexture* Icon;

	UPROPERTY(EditAnywhere)
	float BaseDamage;

	UPROPERTY(EditAnywhere)
	float InitialSpeed;

	UPROPERTY(EditAnywhere)
	float MaxSpeed;

	UPROPERTY(EditAnywhere)
	float Gravity;

	UPROPERTY(EditAnywhere)
	FProjectileEffectsData Effects;

};


UCLASS()
class RPGTEST_API ARPGProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARPGProjectile();

	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
		USphereComponent* CollisionComp;

	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
		UParticleSystemComponent* ParticleComp;

	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
		UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
		USphereComponent* AOECollisionComp;
	
	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		UProjectileMovementComponent* MovementComp;

	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		UDataTable* ProjectileDataTable;

	//TODO: use FDataTableRowHandle instead
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
		FName ProjectileRowName;

	FProjectileData* ProjectileData;
	FProjectileEffectsData ProjectileEffectsData;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
