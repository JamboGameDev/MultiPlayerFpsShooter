// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponData.h"
#include "GameFramework/Actor.h"
#include "FpsWeaponBase.generated.h"


UCLASS()
class FPSSHOOTER_API AFpsWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AFpsWeaponBase();
	
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
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// Компоненты нашего оружия
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* SceneRoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonWeaponMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ThirdPersonWeaponMesh;

	// Указатели на модельки персонажа от 1 и 3 лица
	// Нужны для вызова анимаций персонажа
	UPROPERTY()
	USkeletalMeshComponent* FirstPersonCharacterMesh;
	UPROPERTY()
	USkeletalMeshComponent* ThirdPersonCharacterMesh;

	// Загруженные из таблицы дефолтные настройки оружия
	FWeaponData WeaponData;

	// Рабочие переменные для работы оружия
	float FireTimer = 0.0f;
	float ReloadTime = 1.0f; // заполняем один раз вместо постоянного обращения к анимации
	float RateOfFire = 1.0f; // время между выстрелами, рассчитываем один раз
	FTimerHandle ReloadTimerHandle; // Таймер заводим при начале перезарядки и по его окончанию заканчиваем перезарядку

	// Рабочие функции для работы оружия
	bool GetWeaponDataByID(const FName WeaponID, FWeaponData& OutWeaponData) const;
	void FireTick(const float DeltaTime);

	// Реплицируемые переменные (состояния оружия)
	UPROPERTY(Replicated)
	bool bIsFiring = false; // Начав стрельбу включаем булевую, закончив выключаем
	UPROPERTY(Replicated)
	bool bIsBurstFiring = false; // Очередь в процессе
	UPROPERTY(Replicated)
	bool bIsReloading = false; // Оружие на перезарядке
	UPROPERTY(ReplicatedUsing = OnRep_CurrentAmmo)
	int8 CurrentAmmo;

	// Сетевые функции
	UFUNCTION()
	void OnRep_CurrentAmmo();
	
};
