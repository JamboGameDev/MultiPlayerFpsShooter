// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/FpsWeaponBase.h"

#include "Framework/FpsBaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"


AFpsWeaponBase::AFpsWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>("SceneRoot");

	FirstPersonWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonWeaponMesh"));
	FirstPersonWeaponMesh->SetupAttachment(SceneRoot);
	
	ThirdPersonWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThirdPersonWeaponMesh"));
	ThirdPersonWeaponMesh->SetupAttachment(SceneRoot);

}

void AFpsWeaponBase::ChangeFireStatus(const bool bNewFireStatus)
{
	if (HasAuthority())
	{
		ChangeFireStatus_OnServer(bNewFireStatus);
	}
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
	
	RateOfFire = FMath::Max(0.001f, 60.f / WeaponData.BulletsPerMinute);
	CurrentAmmo = WeaponData.MaxAmmoInClip;
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
				ServerReloadRequest();
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
				Fire_OnServer();
				CurrentAmmo--;
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

void AFpsWeaponBase::ResetAmmo()
{
	CurrentAmmo = WeaponData.MaxAmmoInClip;
}

void AFpsWeaponBase::FinishReloading()
{
	bIsReloading = false;
	ResetAmmo();
}

bool AFpsWeaponBase::IsOwnerLocalPlayer() const
{
	{
		const APawn* OwnerPawn = Cast<APawn>(GetOwner());
		return OwnerPawn && OwnerPawn->IsLocallyControlled();
	} 
}

void AFpsWeaponBase::OnRep_CurrentAmmo() const
{
	// Патроны изменились, например вызов делегата на клиенте
}

void AFpsWeaponBase::Multicast_PlayMontageReload_Implementation()
{
	if (FirstPersonCharacterMesh && FirstPersonCharacterMesh->GetAnimInstance() && FirstPersonFireAnimation)
	{
		FirstPersonCharacterMesh->GetAnimInstance()->Montage_Play(FirstPersonReloadAnimation);
	}
}

void AFpsWeaponBase::ServerReloadRequest_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Server: ServerReloadRequest_Implementation called"));
	if (bIsReloading) return;
	
	bIsReloading = true;
	UE_LOG(LogTemp, Warning, TEXT("Server: Reload started"));
	Multicast_PlayMontageReload();
	
	if (WeaponReloadAnimation)
	{
		ReloadTime = WeaponReloadAnimation->GetPlayLength();
	}
	
	GetWorldTimerManager().SetTimer(ReloadTimerHandle, this, &AFpsWeaponBase::FinishReloading, ReloadTime, false);
}

void AFpsWeaponBase::Multicast_PlayFireAnimation_Implementation()
{
	if (IsOwnerLocalPlayer())
	{
		if (FirstPersonCharacterMesh && FirstPersonCharacterMesh->GetAnimInstance() && FirstPersonFireAnimation)
		{
			FirstPersonCharacterMesh->GetAnimInstance()->Montage_Play(FirstPersonFireAnimation);
		}
		
		if (FirstPersonWeaponMesh && FirstPersonWeaponMesh->GetAnimInstance() && WeaponFireAnimation)
		{
			FirstPersonWeaponMesh->GetAnimInstance()->Montage_Play(WeaponFireAnimation);
		}
	}
	else
	{
		if (ThirdPersonCharacterMesh && ThirdPersonCharacterMesh->GetAnimInstance() && ThirdPersonFireAnimation)
		{
			ThirdPersonCharacterMesh->GetAnimInstance()->Montage_Play(ThirdPersonFireAnimation);
		}
	}
}

void AFpsWeaponBase::Fire_OnServer_Implementation()
{
	if (!ensure(GetOwner() != nullptr)) return;
	
	const auto OwnerPlayer = Cast<AFpsBaseCharacter>(GetOwner());
	const auto OwningController = Cast<APlayerController>(OwnerPlayer->GetController());
	if (!OwnerPlayer || !OwningController) return;

	const FVector FireStartPoint = OwnerPlayer->GetCameraLocation();
	const FVector FireForwardVector = UKismetMathLibrary::GetForwardVector(OwnerPlayer->GetControlRotation_Rep());
	const FRotator BaseRotation = UKismetMathLibrary::MakeRotFromX(FireForwardVector); //Посчитать разброс BaseRotation

	UWorld* World = GetWorld();
	
	FHitResult Hit;
	const FVector TraceEnd = FireStartPoint + BaseRotation.Vector() * 10000.f;

	World->LineTraceSingleByChannel(
		Hit,
		FireStartPoint,
		TraceEnd,
		ECC_GameTraceChannel2, // Используем ваш канал
		FCollisionQueryParams(SCENE_QUERY_STAT(WeaponTrace), true, OwnerPlayer)
	);
	

	if (Hit.bBlockingHit)
	{
		Multicast_PlayHitEffects(Hit);

		if (Hit.GetComponent() && Hit.GetComponent()->IsSimulatingPhysics())
		{
			Hit.GetComponent()->AddImpulseAtLocation(
				BaseRotation.Vector() * 1000000.f,
				Hit.Location,
				Hit.BoneName
			);
		}
		else
		{
			UGameplayStatics::ApplyPointDamage(
				Hit.GetActor(),
				WeaponData.Damage,
				BaseRotation.Vector(),
				Hit,
				OwningController,
				this,
				nullptr
			);
		}
	}

	Multicast_PlayFireAnimation_Implementation();

	// Визуализация трейса
	if (World)
	{
		// Цвет линии, если есть попадание
		FColor LineColor = Hit.bBlockingHit ? FColor::Red : FColor::Green;

		// Рисуем линию трейса
		DrawDebugLine(
			World,
			FireStartPoint,
			Hit.bBlockingHit ? Hit.ImpactPoint : TraceEnd, // Конечная точка зависит от наличия попадания
			LineColor,
			false, // PersistentLines: если true, линия будет отображаться постоянно
			5.0f,  // Duration: время отображения линии (в секундах)
			0,     // DepthPriority
			1.0f   // Thickness: толщина линии
		);
	}

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("FireTick"));
}

void AFpsWeaponBase::ChangeFireStatus_OnServer_Implementation(const bool bNewFireStatus)
{
	bIsFiring = bNewFireStatus;
}

void AFpsWeaponBase::Multicast_PlayHitEffects_Implementation(const FHitResult& Hit)
{
	if (IsOwnerLocalPlayer())
	{
		// Спецэффекты для первого лица (например, следы от пуль вблизи)
	}
	else
	{
		// Вызов Blueprint-ивента
		// Стандартные эффекты для третьего лица
		//HitEffects_BP(HitResult);
	}
}
