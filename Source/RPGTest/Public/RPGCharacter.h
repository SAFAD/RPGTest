// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RPGCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class ARPGProjectile;

UCLASS()
class RPGTEST_API ARPGCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARPGCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
		UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
		USpringArmComponent* SpringArmComp;

	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		FName ProjectileAttachmentSocketName = "ProjectileSocket";

	/** projectile class */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		TSubclassOf<class ARPGProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = Projetile)
	TArray<TSubclassOf<ARPGProjectile>> Projectiles;

	UPROPERTY(Transient, Replicated)
	TArray<TSubclassOf<ARPGProjectile>> Inventory;
	
	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentProjectile)
	TSubclassOf<ARPGProjectile> CurrentProjectile;

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void Shoot();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerShoot();

	
	/** [server] spawns default inventory */
	void SpawnDefaultInventory();

	/**
	* [server] adds a projectile to inventory
	* @param ProjectileClass	Projectile to add
	*/
	void AddProjectile(TSubclassOf<ARPGProjectile> Projectile);

	/**
	* [server] removes projectile to inventory
	* @param ProjectileClass Projectile to remove
	*/
	void RemoveProjectile(TSubclassOf<ARPGProjectile> Projectile);
	
	/**
	* [server + local] equips projectile from inventory
	*
	* @param Projectile	Projectile to equip
	*/
	void EquipProjectile(TSubclassOf<ARPGProjectile> Projectile);

	void EquipNextProjectile();

	UFUNCTION(BlueprintImplementableEvent)
	void OnUnequipProjectile();

	/** equip projectile */
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerEquipProjectile(TSubclassOf<ARPGProjectile> NewProjectile);

	/** updates current projectile */
	void SetCurrentProjectile(TSubclassOf<ARPGProjectile> NewProjectile, TSubclassOf<ARPGProjectile> LastProjectile = NULL);
	/** current projectile rep handler */
	UFUNCTION()
	void OnRep_CurrentProjectile(TSubclassOf<ARPGProjectile> LastProjectile);



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
