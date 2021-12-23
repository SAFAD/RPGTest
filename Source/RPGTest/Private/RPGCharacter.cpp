// Fill out your copyright notice in the Description page of Project Settings.


#include "RPGCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "RPGProjectile.h"


// Sets default values
ARPGCharacter::ARPGCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);


	SpringArmComp->bUsePawnControlRotation = false;
	SpringArmComp->bDoCollisionTest = false;
	SpringArmComp->bInheritPitch = false;
	SpringArmComp->bInheritRoll = false;
	SpringArmComp->bInheritYaw = false;

	SpringArmComp->TargetArmLength = 800.0f;
	SpringArmComp->SetWorldRotation(FRotator(-45.0f, 0.0f, 0.0f));

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

}


void ARPGCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// only to local owner: projectile change requests are locally instigated, other clients don't need it
	DOREPLIFETIME_CONDITION(ARPGCharacter, Inventory, COND_OwnerOnly);

	//everyone
	DOREPLIFETIME(ARPGCharacter, CurrentProjectile);

}

// Called when the game starts or when spawned
void ARPGCharacter::BeginPlay()
{
	Super::BeginPlay();

	SpawnDefaultInventory();

}

void ARPGCharacter::MoveForward(float Value)
{
	AddMovementInput(UKismetMathLibrary::GetForwardVector(FRotator(0.0f, GetControlRotation().Yaw, 0.0f)), Value);
}

void ARPGCharacter::MoveRight(float Value)
{
	AddMovementInput(UKismetMathLibrary::GetRightVector(FRotator(0.0f, GetControlRotation().Yaw, 0.0f)), Value);
}

void ARPGCharacter::Shoot()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerShoot();
		return;
	}

	ServerHandleCooldown(CurrentProjectile);

	if (!bCanShoot)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, TEXT("Cooldown!"));
		return;
	}

	if (!CurrentProjectile)
	{
		UE_LOG(LogTemp, Error, TEXT("Current Projectile isn't set properly, due to the projectile not having a datatable set"));
		return;
	}
	if (UWorld* const World = GetWorld())
	{
		if (AController* MyInstigator = GetInstigatorController()) {


			const FRotator SpawnRotation = GetControlRotation();
			

			const FVector SpawnLocation = GetMesh()->GetSocketLocation(ProjectileAttachmentSocketName);// + SpawnRotation.Vector();

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.Instigator = this;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;


			FTransform SpawnTransform(GetActorForwardVector().Rotation(), SpawnLocation);
			ARPGProjectile* SpawnedProjectile = World->SpawnActor<ARPGProjectile>(CurrentProjectile, SpawnTransform, ActorSpawnParams);

			//it must have LastFired Time
			if (!LastFired.Contains(CurrentProjectile))
			{
				LastFired.Add(CurrentProjectile, World->TimeSeconds);
			}
			if (!Cooldowns.Contains(CurrentProjectile))
			{
				Cooldowns.Add(CurrentProjectile, SpawnedProjectile->ProjectileData->Cooldown);
			}

			LastFired[CurrentProjectile] = World->TimeSeconds;

			OnShootProjectile.Broadcast(SpawnedProjectile);

		}
	}

}

void ARPGCharacter::ServerShoot_Implementation()
{
	Shoot();
}

bool ARPGCharacter::ServerShoot_Validate()
{
	//TODO: get this done
	return true;
}


void ARPGCharacter::ServerHandleCooldown_Implementation(TSubclassOf<ARPGProjectile> Projectile)
{
	// if neither LastFired OR Cooldown is set, just return true and it will be set after projectile is shot
	if (!LastFired.Contains(Projectile) || !Cooldowns.Contains(Projectile))
	{
		bCanShoot = true;
		return;
	}
	float ProjectileLastFired = LastFired[Projectile];
	float ProjectileCooldown = Cooldowns[Projectile];


	if (GetWorld()->TimeSeconds > ProjectileLastFired + ProjectileCooldown)
	{
		bCanShoot = true;
		OnProjectileCooldownEnd.Broadcast(Projectile);
		return;
	}


	bCanShoot = false;
}

void ARPGCharacter::SpawnDefaultInventory()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		return;
	}

	int32 NumProjectileClasses = Projectiles.Num();
	for (int32 i = 0; i < NumProjectileClasses; i++)
	{
		if (TSubclassOf<ARPGProjectile> NewProjectile = Projectiles[i])
		{
			AddProjectile(NewProjectile);
		}
	}

	// equip first projectile in inventory
	if (Inventory.Num() > 0)
	{
		EquipProjectile(Inventory[0]);
	}
}

void ARPGCharacter::AddProjectile(TSubclassOf<ARPGProjectile> Projectile)
{
	if (Projectile && GetLocalRole() == ROLE_Authority)
	{
		Inventory.AddUnique(Projectile);
	}
}

void ARPGCharacter::RemoveProjectile(TSubclassOf<ARPGProjectile> Projectile)
{
	if (Projectile && GetLocalRole() == ROLE_Authority)
	{
		Inventory.RemoveSingle(Projectile);
	}
}

void ARPGCharacter::EquipProjectile(TSubclassOf<ARPGProjectile> Projectile)
{
	if (Projectile)
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			SetCurrentProjectile(Projectile, CurrentProjectile);
		}
		else
		{
			ServerEquipProjectile(Projectile);
		}
	}
}

void ARPGCharacter::EquipNextProjectile()
{
	if (Inventory.Num() > 0)
	{

		const int32 CurrentProjectileIdx = Inventory.IndexOfByKey(CurrentProjectile);

		TSubclassOf<ARPGProjectile> NextProjectile = Inventory[(CurrentProjectileIdx + 1) % Inventory.Num()];
		//if we're equipping last projectile, go to first
		if (CurrentProjectileIdx == Inventory.Num() - 1)
		{
			NextProjectile = Inventory[0];
			EquipProjectile(NextProjectile);
			return;
		}
		//else proceed with usual
		EquipProjectile(NextProjectile);

		UE_LOG(LogTemp, Warning, TEXT("New Projectile: %s"), *NextProjectile->GetName());
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Empty Inventory!"));
	}
}

void ARPGCharacter::ServerEquipProjectile_Implementation(TSubclassOf<ARPGProjectile> NewProjectile)
{
	EquipProjectile(NewProjectile);
}

bool ARPGCharacter::ServerEquipProjectile_Validate(TSubclassOf<ARPGProjectile> NewProjectile)
{
	//TODO: implement this
	return true;
}

void ARPGCharacter::SetCurrentProjectile(TSubclassOf<ARPGProjectile> NewProjectile, TSubclassOf<ARPGProjectile> LastProjectile /*= NULL*/)
{
	TSubclassOf<ARPGProjectile> LocalLastProjectile = nullptr;

	if (LastProjectile != NULL)
	{
		LocalLastProjectile = LastProjectile;
	}
	else if (NewProjectile != CurrentProjectile)
	{
		LocalLastProjectile = CurrentProjectile;
	}

	// unequip previous
	if (LocalLastProjectile)
	{
		OnUnEquipProjectile.Broadcast(LocalLastProjectile);
	}

	CurrentProjectile = NewProjectile;

	OnEquipProjectile.Broadcast(CurrentProjectile);
}

void ARPGCharacter::OnRep_CurrentProjectile(TSubclassOf<ARPGProjectile> LastProjectile)
{
	SetCurrentProjectile(CurrentProjectile, LastProjectile);
}

// Called every frame
void ARPGCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ARPGCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ARPGCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ARPGCharacter::MoveRight);

	// Bind Shooting event

	PlayerInputComponent->BindAction("Shoot", IE_Released, this, &ARPGCharacter::Shoot);

	// Bind switching projectiles event
	PlayerInputComponent->BindAction("NextProjectile", IE_Released, this, &ARPGCharacter::EquipNextProjectile);

}

