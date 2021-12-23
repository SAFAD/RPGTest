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
	TSubclassOf<class UDamageType> DamageTypeClass;

	UPROPERTY(EditAnywhere)
	float InitialSpeed;

	UPROPERTY(EditAnywhere)
	float MaxSpeed;

	UPROPERTY(EditAnywhere)
	float Gravity;

	UPROPERTY(EditAnywhere)
	bool bDestroyOnImpact;

	UPROPERTY(EditAnywhere)
	bool bIsAoe;

	UPROPERTY(EditAnywhere)
	int AoeCap;

	UPROPERTY(EditAnywhere)
	float Cooldown;

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
	FDataTableRowHandle ProjectileDataRow;

	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void ApplyAoeImpact();
	
	UFUNCTION()
	void ApplyImpact(UPrimitiveComponent* OtherComp);

	UFUNCTION()
	void DestroyActor(AActor* Actor);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDestroyActor(AActor* Actor);

	UFUNCTION(NetMulticast, Reliable)
	void MultiCastPlayImpactEffect();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FProjectileData* ProjectileData;
	FProjectileEffectsData ProjectileEffectsData;
};
