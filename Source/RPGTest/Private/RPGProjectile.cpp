// Fill out your copyright notice in the Description page of Project Settings.

#include "RPGProjectile.h"
#include "../RPGTest.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ARPGProjectile::ARPGProjectile()
{
	
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->AlwaysLoadOnClient = true;
	CollisionComp->AlwaysLoadOnServer = true;
	//TODO: Fix collision with owner
	CollisionComp->IgnoreActorWhenMoving(GetInstigator(), true);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionObjectType(COLLISION_PROJECTILE);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Ignore);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &ARPGProjectile::OnHit);
	RootComponent = CollisionComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetupAttachment(RootComponent);

	ParticleComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleComp"));
	ParticleComp->SetupAttachment(RootComponent);

	// Use a ProjectileMovementComponent to govern this projectile's movement
	MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	MovementComp->UpdatedComponent = CollisionComp;
	MovementComp->InitialSpeed = 3000.f;
	MovementComp->MaxSpeed = 3000.f;
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->bShouldBounce = false;

	AOECollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("AOESphereComp"));
	AOECollisionComp->InitSphereRadius(300.0f);
	AOECollisionComp->AlwaysLoadOnClient = true;
	AOECollisionComp->AlwaysLoadOnServer = true;
	AOECollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AOECollisionComp->SetCollisionObjectType(COLLISION_PROJECTILE);
	AOECollisionComp->SetCollisionResponseToAllChannels(ECR_Overlap);
	AOECollisionComp->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Ignore);
	AOECollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	AOECollisionComp->SetupAttachment(RootComponent);
	// Die after 10 seconds by default, don't want this to last forever as it will eventually crash the server
	InitialLifeSpan = 10.0f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	SetReplicatingMovement(true);
}

// Called when the game starts or when spawned
void ARPGProjectile::BeginPlay()
{
	Super::BeginPlay();
	

	
}

void ARPGProjectile::PostInitializeComponents()
{
	if (!ProjectileDataRow.IsNull())
	{
		ProjectileData = ProjectileDataRow.DataTable->FindRow<FProjectileData>(ProjectileDataRow.RowName, ProjectileDataRow.RowName.ToString());
		MovementComp->InitialSpeed = ProjectileData->InitialSpeed;
		MovementComp->MaxSpeed = ProjectileData->MaxSpeed;
		MovementComp->ProjectileGravityScale = ProjectileData->Gravity;

		ProjectileEffectsData = ProjectileData->Effects;
		ParticleComp->SetTemplate(ProjectileEffectsData.BaseEffect);

	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Either the Datatable or the Row name is undefined"));
	}
}

void ARPGProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

	if (OtherActor != nullptr && OtherActor != this && OtherActor != GetInstigator())
	{
		if (ProjectileData->bIsAoe) {
			ApplyAoeImpact();
		}
		else {
			ApplyImpact(OtherComp);
		}
	}
}

void ARPGProjectile::ApplyAoeImpact()
{
	TArray<UPrimitiveComponent*> OverlappedComponents;
	int i = 0;

	AOECollisionComp->GetOverlappingComponents(OverlappedComponents);

	for (UPrimitiveComponent* OverlappedComponent : OverlappedComponents)
	{
		if (i > ProjectileData->AoeCap)
		{
			break;
		}
		ApplyImpact(OverlappedComponent);
		i++;
	}
}

void ARPGProjectile::ApplyImpact(UPrimitiveComponent* OtherComp)
{
	AActor* OtherActor = OtherComp->GetOwner();
	switch (OtherComp->GetCollisionObjectType()) {
	case ECC_Pawn:
		UGameplayStatics::ApplyDamage(OtherActor, ProjectileData->BaseDamage, GetInstigatorController(), GetInstigator(), ProjectileData->DamageTypeClass);
	case ECC_Destructible:
		OtherActor->Destroy();
	case ECC_WorldStatic:
		UE_LOG(LogTemp, Warning, TEXT("IS STATIC DO NOTHING"));
	}

	MultiCastPlayImpactEffect();
	
	if (ProjectileData->bDestroyOnImpact)
	{
		Destroy();
	}
}

void ARPGProjectile::DestroyActor(AActor* Actor)
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerDestroyActor(Actor);
		return;
	}

	Actor->Destroy();
}

void ARPGProjectile::ServerDestroyActor_Implementation(AActor* Actor)
{
	DestroyActor(Actor);
}

bool ARPGProjectile::ServerDestroyActor_Validate(AActor* Actor)
{
	//TODO: Implement this
	return true;
}

void ARPGProjectile::MultiCastPlayImpactEffect_Implementation()
{

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ProjectileEffectsData.ImpactEffect, GetActorTransform());
}

// Called every frame
void ARPGProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

