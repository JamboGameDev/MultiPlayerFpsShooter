

#pragma once

#include "CoreMinimal.h"
#include "WeaponData.generated.h"

class AFpsBulletBase;
class AFpsWeaponBase;

UENUM(BlueprintType)
enum class EWeaponFireMode : uint8
{
	Single UMETA(DisplayName = "Одиночными"),
	Burst UMETA(DisplayName = "Очередь"),
	Auto UMETA(DisplayName = "Автоматический")
};

USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

	// Уникальный идентификатор оружия
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
	FName WeaponID;

	// Название оружия
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
	FText DisplayName;

	// Класс оружия для спавна
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
	TSubclassOf<AFpsWeaponBase> WeaponClass;

	// Классс пули
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
	TSubclassOf<AFpsBulletBase> ProjectileClass;
	
	// Режим стрельбы
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	EWeaponFireMode FireMode = EWeaponFireMode::Auto;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings",
	meta = (
		EditCondition = "FireMode == EWeaponFireMode::Burst",
		EditConditionHides,
		ClampMin = 1,     
		UIMin = 1,          
		ClampMax = 10,      
		UIMax = 10          
	))
	int32 BurstShotsCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings",
		meta = (
			EditCondition = "FireMode == EWeaponFireMode::Burst",
			EditConditionHides,
			ClampMin = "0.01",  
			UIMin = "0.01",     
			ClampMax = "1.0", 
			UIMax = "1.0"
		))
	float BurstDelay = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bIsShotgun = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings",
		meta = (
			EditCondition = "bIsShotgun",
			EditConditionHides,
			ClampMin = 1,       
			UIMin = 1,
			ClampMax = 20,       
			UIMax = 20
		))
	int32 BulletsOnShoot = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings",
	meta = (
		ClampMin = 1,   
		UIMin = 1,
		ClampMax = 127,   // Максимальный разброс (градусы)
		UIMax = 127
	))
	int32 MaxAmmoInClip = 30;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float BulletsPerMinute = 600.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float Damage = 10.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings",
		meta = (
			ClampMin = "0.1",   
			UIMin = "0.1",
			ClampMax = "10.0",   // Максимальный разброс (градусы)
			UIMax = "10.0"
		))
	float SpreadAngle = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Additional")
	FName AttachmentSocketName = "weapon_rSocket";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Additional")
	FName MuzzleSocketName = "muzzle_joint";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Additional")
	FVector MuzzleOffset = FVector(0.0f, 0.0f, 0.0f);
	
};
