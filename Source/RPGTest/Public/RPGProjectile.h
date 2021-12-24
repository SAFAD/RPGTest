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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UParticleSystem* BaseEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UParticleSystem* ImpactEffect;

};

USTRUCT(BlueprintType)
struct FProjectileData : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UDamageType> DamageTypeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InitialSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Gravity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDestroyOnImpact;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAoe;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int AoeCap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
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


	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		FDataTableRowHandle ProjectileDataRow;

	FProjectileData* ProjectileData;
	FProjectileEffectsData ProjectileEffectsData;
};
