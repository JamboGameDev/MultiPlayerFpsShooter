// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/FpsBaseCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/HealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"


AFpsBaseCharacter::AFpsBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Создаем компонент камеры
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetRootComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.f, 0.f, 64.f)); // Примерная высота глаз
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Создаем меш для первого лица (руки)
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMesh->SetupAttachment(FirstPersonCameraComponent);
	FirstPersonMesh->SetOnlyOwnerSee(true); // Виден только владельцу
	FirstPersonMesh->CastShadow = false;
	FirstPersonMesh->bCastDynamicShadow = false;
	
	// Для удобства сохраняем ссылку на ThirdPersonMesh
	ThirdPersonMesh = GetMesh();

	// Настройка меша для третьего лица (тело)
	ThirdPersonMesh->SetOwnerNoSee(true); // Владелец не видит этот меш
	ThirdPersonMesh->SetupAttachment(GetRootComponent());
	ThirdPersonMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	ThirdPersonMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void AFpsBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFpsBaseCharacter, CurrentWeapon);
	DOREPLIFETIME(AFpsBaseCharacter, ControlRotation_Rep);
	DOREPLIFETIME(AFpsBaseCharacter, CameraLocation_Rep);
}

void AFpsBaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HealthComponent)
	{
		OnTakeAnyDamage.AddDynamic(HealthComponent, &UHealthComponent::HandleAnyDamage);
		OnTakePointDamage.AddDynamic(HealthComponent, &UHealthComponent::HandlePointDamage);
		OnTakeRadialDamage.AddDynamic(HealthComponent, &UHealthComponent::HandleRadialDamage);
		
		HealthComponent->OnDeath.BindUObject(this, &AFpsBaseCharacter::OnCharacterDied);
	}
}

void AFpsBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		InitWeapon_OnServer();
	}
}

void AFpsBaseCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetController() && (IsLocallyControlled() || HasAuthority()) && FirstPersonCameraComponent)
	{
		ControlRotation_Rep = GetController()->GetControlRotation();
		CameraLocation_Rep = FirstPersonCameraComponent->GetComponentLocation();
	}

	if (!HealthComponent->IsAlive())
	{
		if (FirstPersonCameraComponent && ThirdPersonMesh)
		{
			FirstPersonCameraComponent->bUsePawnControlRotation = false;
		
			FirstPersonCameraComponent->SetWorldRotation(UKismetMathLibrary::FindLookAtRotation(
				FirstPersonCameraComponent->GetComponentLocation(), ThirdPersonMesh->GetComponentLocation()));
		}
	}
}

void AFpsBaseCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	
	// Добавляем контекст ввода для локального игрока
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AFpsBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// Настраиваем Enhanced Input
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFpsBaseCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFpsBaseCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AFpsBaseCharacter::StartJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AFpsBaseCharacter::StopJump);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AFpsBaseCharacter::ToggleCrouch);
		EnhancedInputComponent->BindAction(Fire, ETriggerEvent::Started, this, &AFpsBaseCharacter::FireStart);
		EnhancedInputComponent->BindAction(Fire, ETriggerEvent::Completed, this, &AFpsBaseCharacter::FireStop);
	}
}

void AFpsBaseCharacter::InitWeapon_OnServer_Implementation()
{
	if (!ensure(WeaponClass)) return;
	
	const FTransform SpawnTransform = GetActorTransform();
	CurrentWeapon = GetWorld()->SpawnActor<AFpsWeaponBase>(WeaponClass, SpawnTransform);
	if (CurrentWeapon)
	{
		CurrentWeapon->SetOwner(this);
		CurrentWeapon->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		CurrentWeapon->AttachWeaponMeshes(FirstPersonMesh, ThirdPersonMesh);
	}
}

void AFpsBaseCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AFpsBaseCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AFpsBaseCharacter::StartJump()
{
	Jump();
}

void AFpsBaseCharacter::StopJump()
{
	StopJumping();
}

void AFpsBaseCharacter::ToggleCrouch()
{
	if (GetCharacterMovement()->IsCrouching())
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AFpsBaseCharacter::UpdateMeshVisibility(const bool bAlive)
{
	if (IsLocallyControlled())
	{
		// Для локального игрока показываем только руки
		FirstPersonMesh->SetOnlyOwnerSee(bAlive);
		FirstPersonMesh->SetVisibility(bAlive);
	
		ThirdPersonMesh->SetOwnerNoSee(bAlive);
		ThirdPersonMesh->SetVisibility(!bAlive);
	}
	else
	{
		// Для других игроков показываем только тело
		FirstPersonMesh->SetVisibility(false);
		ThirdPersonMesh->SetVisibility(true);
	}
}

void AFpsBaseCharacter::FireStart()
{
	if (CurrentWeapon) CurrentWeapon->ChangeFireStatus(true);
}

void AFpsBaseCharacter::FireStop()
{
	if (CurrentWeapon) CurrentWeapon->ChangeFireStatus(false);
}

void AFpsBaseCharacter::OnCharacterDied()
{
	// Останавливаем движение
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	// Multicast Может вызывать только сервер
	if (GetLocalRole() == ROLE_Authority)
	{
		Multicast_PlayDeathEffects();
	}
	
	// Выключаем контроллер
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		PC->SetCinematicMode(true, false, true, true, true);
	}

	if (FirstPersonCameraComponent && ThirdPersonMesh)
	{
		FirstPersonCameraComponent->bUsePawnControlRotation = false;
	}
}

void AFpsBaseCharacter::SpawnMuzzle()
{
	
}

void AFpsBaseCharacter::Multicast_PlayDeathEffects_Implementation()
{
	UpdateMeshVisibility(false);
	
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);

	// Можно проиграть анимацию смерти, звук, камеру и т.д.
	//UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
}
