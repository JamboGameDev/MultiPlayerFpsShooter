// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/FpsWeaponBase.h"

#include "Net/UnrealNetwork.h"


AFpsWeaponBase::AFpsWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Заполняем компоненты оружия
}

void AFpsWeaponBase::AttachWeaponMeshes(USkeletalMeshComponent* FirstMeshComp,
	USkeletalMeshComponent* ThirdMeshComp)
{
	// макрос, который предоставляет улучшенную проверку условий
	// с детализированным логированием при ошибках.
	if (!ensure(FirstMeshComp && ThirdMeshComp)) return;
	if (!ensure(FirstPersonWeaponMesh && ThirdPersonWeaponMesh)) return;

	// Сохраняем указатели на компоненты игрока
	FirstPersonCharacterMesh = FirstMeshComp;
	ThirdPersonCharacterMesh = ThirdMeshComp;

	FirstPersonWeaponMesh->AttachToComponent(FirstMeshComp, FAttachmentTransformRules::KeepRelativeTransform, WeaponData.AttachmentSocketName);
	ThirdPersonWeaponMesh->AttachToComponent(ThirdMeshComp, FAttachmentTransformRules::KeepRelativeTransform, WeaponData.AttachmentSocketName);

	// Проиграть анимацию экипирования для персонажа
	if (FirstPersonEquipAnimation && FirstPersonCharacterMesh && FirstPersonCharacterMesh->GetAnimInstance())
		FirstPersonCharacterMesh->GetAnimInstance()->Montage_Play(FirstPersonEquipAnimation);
	if (ThirdPersonEquipAnimation && ThirdPersonCharacterMesh && ThirdPersonCharacterMesh->GetAnimInstance())
		ThirdPersonCharacterMesh->GetAnimInstance()->Montage_Play(ThirdPersonEquipAnimation);

}

void AFpsWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	GetWeaponDataByID(CurrentWeaponID, WeaponData);

	// Заполняем рабочие переменные дефолтными значениями
	RateOfFire = FMath::Max(0.001f, 60.f / WeaponData.BulletsPerMinute);
	CurrentAmmo = WeaponData.MaxAmmoInClip;
	
	if (WeaponReloadAnimation)
	{
		ReloadTime = WeaponReloadAnimation->GetPlayLength();
	}
}

void AFpsWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Вместо таймеров будем использовать тик на сервере
	if (HasAuthority())
		FireTick(DeltaTime);
}

void AFpsWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AFpsWeaponBase, bIsFiring, COND_SkipOwner);
	DOREPLIFETIME(AFpsWeaponBase, bIsBurstFiring);
	DOREPLIFETIME_CONDITION(AFpsWeaponBase, bIsReloading, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AFpsWeaponBase, CurrentAmmo, COND_OwnerOnly);
}

bool AFpsWeaponBase::GetWeaponDataByID(const FName WeaponID, FWeaponData& OutWeaponData) const
{
	if (!WeaponDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("WeaponDataTable is null!"));
		return false;
	}

	// Ищем строку в таблице данных
	const FWeaponData* FoundRow = WeaponDataTable->FindRow<FWeaponData>(WeaponID, "");
	if (FoundRow)
	{
		OutWeaponData = *FoundRow; // Копируем данные из указателя
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("Weapon data for ID %s not found!"), *WeaponID.ToString());
	return false;
}

void AFpsWeaponBase::FireTick(const float DeltaTime)
{
	if (!HasAuthority()) return;

	if (bIsFiring && !bIsBurstFiring && !bIsReloading)
	{
		// Общая проверка таймера для всех режимов
		if (FireTimer <= 0.0f)
		{
			// Проверка боеприпасов
			if (CurrentAmmo <= 0)
			{
				bIsFiring = false;
				//ReloadRequest();
				return;
			}

			// Обработка разных режимов
			switch (WeaponData.FireMode)
			{
			case EWeaponFireMode::Single:
				//Fire_OnServer();
				bIsFiring = false; // Сброс после выстрела
				break;
			case EWeaponFireMode::Burst:
				//StartBurstFire();
				break;
			case EWeaponFireMode::Auto:
				//Fire_OnServer();
				break;
			}
        
			FireTimer = RateOfFire;
		}
		else
		{
			FireTimer -= DeltaTime;
		}
	}
}

void AFpsWeaponBase::OnRep_CurrentAmmo() const
{
	// Патроны изменились, например вызов делегата на клиенте
}
