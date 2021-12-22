// Fill out your copyright notice in the Description page of Project Settings.


#include "RPGCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "RPGProjectile.h"


// Sets default values
ARPGCharacter::ARPGCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	this->bUseControllerRotationYaw = false;
	

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

// Called when the game starts or when spawned
void ARPGCharacter::BeginPlay()
{
	Super::BeginPlay();
	
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
	
	if (UWorld* const World = GetWorld())
	{
		if (AController* MyInstigator = GetInstigatorController()) {
			const FRotator SpawnRotation = MyInstigator->GetControlRotation();
			
			const FVector SpawnLocation = GetMesh()->GetSocketLocation(ProjectileAttachmentSocketName) + SpawnRotation.Vector();

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


			FTransform SpawnTransform(SpawnRotation, SpawnLocation);
			World->SpawnActor<ARPGProjectile>(ProjectileClass, SpawnTransform, ActorSpawnParams);

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

}

