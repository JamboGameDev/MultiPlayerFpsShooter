// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponData.h"
#include "GameFramework/Actor.h"
#include "FpsWeaponBase.generated.h"

class USkeletalMeshComponent;

// Абстрактный класс оружия, нельзя спавнить
UCLASS(Abstract, Blueprintable)
class FPSSHOOTER_API AFpsWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AFpsWeaponBase();

	void ChangeFireStatus(const bool bNewFireStatus);
	
	UFUNCTION()
	void AttachWeaponMeshes(USkeletalMeshComponent* FirstMeshComp, USkeletalMeshComponent* ThirdMeshComp);

	UPROPERTY(EditDefaultsOnly, Category = "Settings | Base")
	FName CurrentWeaponID = "Default";
	
	UPROPERTY(EditDefaultsOnly, Category = "Settings | Base")
	UDataTable* WeaponDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Settings | WeaponAnimation")
	UAnimMontage* WeaponFireAnimation;
	
	UPROPERTY(EditDefaultsOnly, Category = "Settings | WeaponAnimation")
	UAnimMontage* WeaponReloadAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Settings | FirstPersonAnimation")
	UAnimMontage* FirstPersonEquipAnimation;
	
	UPROPERTY(EditDefaultsOnly, Category = "Settings | FirstPersonAnimation")
	UAnimMontage* FirstPersonFireAnimation;
	
	UPROPERTY(EditDefaultsOnly, Category = "Settings | FirstPersonAnimation")
	UAnimMontage* FirstPersonReloadAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Settings | ThirdPersonAnimation")
	UAnimMontage* ThirdPersonEquipAnimation;
	
	UPROPERTY(EditDefaultsOnly, Category = "Settings | ThirdPersonAnimation")
	UAnimMontage* ThirdPersonFireAnimation;
	
	UPROPERTY(EditDefaultsOnly, Category = "Settings | ThirdPersonAnimation")
	UAnimMontage* ThirdPersonReloadAnimation;

	UFUNCTION(Server, Reliable)
	void ServerReloadRequest();

	UPROPERTY(Replicated)
	bool bIsReloading = false; // Оружие на перезарядке
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* SceneRoot;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonWeaponMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ThirdPersonWeaponMesh;

	
	UPROPERTY()
	USkeletalMeshComponent* FirstPersonCharacterMesh;
	
	UPROPERTY()
	USkeletalMeshComponent* ThirdPersonCharacterMesh;

	
	FWeaponData WeaponData;

	// Рабочие переменные для работы оружия
	float FireTimer = 0.0f;
	float ReloadTime = 2.0f;
	float RateOfFire = 1.0f;
	FTimerHandle FireBurstTimerHandle; 
	int32 CurrentBurstShotIndex = 0; 
	FTimerHandle ReloadTimerHandle; 

	// Рабочие функции для работы оружия
	bool GetWeaponDataByID(const FName WeaponID, FWeaponData& OutWeaponData) const;
	void FireTick(const float DeltaTime);
	void ResetAmmo();
	void FinishReloading();

	bool IsOwnerLocalPlayer() const;

	// Реплицируемые переменные (состояния оружия)
	UPROPERTY(Replicated)
	bool bIsFiring = false;
	
	UPROPERTY(Replicated)
	bool bIsBurstFiring = false; 
	
	UPROPERTY(ReplicatedUsing = OnRep_CurrentAmmo)
	int32 CurrentAmmo;

	// Сетевые функции
	UFUNCTION()
	void OnRep_CurrentAmmo() const;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHitEffects(const FHitResult& Hit);

	UFUNCTION(Server, Reliable)
	void ChangeFireStatus_OnServer(const bool bNewFireStatus);

	UFUNCTION(Server, Reliable)
	void Fire_OnServer();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayFireAnimation();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayMontageReload();
};
