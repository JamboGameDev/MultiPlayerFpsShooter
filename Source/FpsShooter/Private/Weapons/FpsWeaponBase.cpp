// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/FpsWeaponBase.h"

#include "Framework/FpsBaseCharacter.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AFpsWeaponBase::AFpsWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

}

void AFpsWeaponBase::AttachWeaponMeshes(USkeletalMeshComponent* FirstPersonMesh,
	USkeletalMeshComponent* ThirdPersonMesh)
{
	const auto OwnerPlayer = Cast<AFpsBaseCharacter>(GetOwner());
	const auto OwningController = Cast<APlayerController>(OwnerPlayer->GetController());
	if (!OwnerPlayer || !OwningController) return;


	const FVector FireStartPoint = OwnerPlayer->GetCameraLocation();
	const FVector FireForwardVector = UKismetMathLibrary::GetForwardVector(OwnerPlayer->GetControlRotation_Rep());
	
	const FRotator BaseRotation = UKismetMathLibrary::MakeRotFromX(FireForwardVector);

}

// Called when the game starts or when spawned
void AFpsWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFpsWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

