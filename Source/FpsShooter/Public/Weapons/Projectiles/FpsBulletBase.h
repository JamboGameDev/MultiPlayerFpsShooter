// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FpsBulletBase.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class FPSSHOOTER_API AFpsBulletBase : public AActor
{
	GENERATED_BODY()
	
public:
	AFpsBulletBase();
	
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	float DefaultLifeSpan = 3.0f;

	// Оружие при спавне будет обновлять урон пули
	FORCEINLINE void SetDamage(const float NewDamage){ Damage = NewDamage; }
	
protected:
	virtual void PostInitializeComponents() override;

	UFUNCTION(BlueprintImplementableEvent)
	void PlayImpactEffects_BP(const FHitResult& Hit);

	// Сетевые функции
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	USphereComponent* CollisionSphere;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

	UFUNCTION()
	void BulletOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void BulletHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(Replicated)
	float Damage = 1;

	UFUNCTION(NetMulticast, Reliable)
	void PlayImpactEffects_Multicast(const FHitResult& Hit);

};
