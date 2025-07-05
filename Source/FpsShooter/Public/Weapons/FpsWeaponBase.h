// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FpsWeaponBase.generated.h"

UENUM(BlueprintType)
enum class EWeaponFireMode : uint8
{
	Single,
	Burst,
	Auto
 };


UCLASS()
class FPSSHOOTER_API AFpsWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AFpsWeaponBase();

	UFUNCTION()
	void AttachWeaponMeshes(USkeletalMeshComponent* FirstPersonMesh, USkeletalMeshComponent* ThirdPersonMesh);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	

};
