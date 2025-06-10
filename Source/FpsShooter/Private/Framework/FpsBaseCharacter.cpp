// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/FpsBaseCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
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
}

void AFpsBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFpsBaseCharacter, ControlRotation_Rep);
}

void AFpsBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AFpsBaseCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetController() && (IsLocallyControlled() || HasAuthority()))
	{
		ControlRotation_Rep = GetController()->GetControlRotation();
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

void AFpsBaseCharacter::UpdateMeshVisibility()
{
	if (IsLocallyControlled())
	{
		// Для локального игрока показываем только руки
		FirstPersonMesh->SetVisibility(true, true);
		ThirdPersonMesh->SetVisibility(false, true);
	}
	else
	{
		// Для других игроков показываем только тело
		FirstPersonMesh->SetVisibility(false, true);
		ThirdPersonMesh->SetVisibility(true, true);
	}
}
