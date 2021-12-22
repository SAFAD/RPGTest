// Fill out your copyright notice in the Description page of Project Settings.

#include "../RPGTest.h"
#include "RPGProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
ARPGProjectile::ARPGProjectile()
{
	
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->AlwaysLoadOnClient = true;
	CollisionComp->AlwaysLoadOnServer = true;
	//CollisionComp->bTraceComplexOnMove = true;
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionObjectType(COLLISION_PROJECTILE);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Ignore);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	//CollisionComp->OnComponentHit.AddDynamic(this, &ARPGProjectile::OnHit);
	RootComponent = CollisionComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetupAttachment(RootComponent);

	ParticleComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleComp"));
	//ParticleComp->SetTemplate(TrailEffect);
	ParticleComp->SetupAttachment(RootComponent);

	// Use a ProjectileMovementComponent to govern this projectile's movement
	MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	MovementComp->UpdatedComponent = CollisionComp;
	MovementComp->InitialSpeed = 3000.f;
	MovementComp->MaxSpeed = 3000.f;
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->bShouldBounce = false;

	AOECollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("AOESphereComp"));
	AOECollisionComp->InitSphereRadius(20.0f);
	AOECollisionComp->AlwaysLoadOnClient = true;
	AOECollisionComp->AlwaysLoadOnServer = true;
	//CollisionComp->bTraceComplexOnMove = true;
	AOECollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AOECollisionComp->SetCollisionObjectType(COLLISION_PROJECTILE);
	AOECollisionComp->SetCollisionResponseToAllChannels(ECR_Overlap);
	AOECollisionComp->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Ignore);
	AOECollisionComp->BodyInstance.SetCollisionProfileName("Projectile");

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;

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

// Called every frame
void ARPGProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

