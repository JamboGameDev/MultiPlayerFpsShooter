// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Weapons/FpsWeaponBase.h"
#include "FpsBaseCharacter.generated.h"

class UCameraComponent;
class UHealthComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class FPSSHOOTER_API AFpsBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFpsBaseCharacter();

	/** Сетевые функции */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Получения реплицированного поворота контроллера (как пример) */
    UFUNCTION(BlueprintPure, BlueprintCallable)
    FORCEINLINE FRotator GetControlRotation_Rep() const { return ControlRotation_Rep;}
	
	UFUNCTION(BlueprintPure, BlueprintCallable)
	FORCEINLINE FVector GetCameraLocation() const { return CameraLocation_Rep;}
	
protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Контекст ввода */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings | Input")
	UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings | Input")
	UInputAction* MoveAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings | Input")
	UInputAction* LookAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings | Input")
	UInputAction* JumpAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings | Input")
	UInputAction* CrouchAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings | Input")
	UInputAction* Fire;

	//------weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSlot")
	TSubclassOf<AFpsWeaponBase> WeaponClass;
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartJump();
	void StopJump();
	void ToggleCrouch();
	void UpdateMeshVisibility(const bool bAlive);
	void FireStart();
	void FireStop();
	
	UFUNCTION(meta = (ToolTip = "Реакция на смерть"))
	void OnCharacterDied();

private:
	/** Компонент камеры */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;
	
	/** Скелетный меш для рук (виден только себе) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;
	
	/** Скелетный меш для тела (виден другим игрокам) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ThirdPersonMesh;

	// Свой компонент здоровья
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UHealthComponent* HealthComponent;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDeathEffects();
	
	UPROPERTY(Replicated);
	AFpsWeaponBase* CurrentWeapon;
	
	UFUNCTION(Server, Reliable)
	void InitWeapon_OnServer();

	void SpawnMuzzle();

	/** Реплицированный поворот камеры */
	UPROPERTY(Replicated)
	FRotator ControlRotation_Rep;
	
	UPROPERTY(Replicated)
	FVector CameraLocation_Rep;
};
