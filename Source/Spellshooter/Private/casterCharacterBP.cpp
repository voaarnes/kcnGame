// Fill out your copyright notice in the Description page of Project Settings.


#include "casterCharacterBP.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Controller.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "bullet.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "playerHUD.h"
#include "humanCharacter.h"

// Sets default values
AcasterCharacterBP::AcasterCharacterBP()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	//SpringArmComp->SetupAttachment(RootComponent);   //this attaches SpringArmComp to the rootComponent which here is the capsule

	//CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	//CameraComp->SetupAttachment(SpringArmComp);

	//MeshComp = CreateAbstractDefaultSubobject<USkeletalMeshComponent>(TEXT("PlayerMesh"));
	///MeshComp->SetupAttachment(RootComponent);

	BaseTurnRate = 45.0f;
	BaseLookUpAtRate = 45.0f;
	bUseControllerRotationYaw = true;

}

// Called when the game starts or when spawned
void AcasterCharacterBP::BeginPlay()
{
	Super::BeginPlay();
	maxPlayerHP = 100.0f;
	currentPlayerHP = maxPlayerHP;
	playerHPpercent = 1.0f;
	playerIsDead = false;	
}

void AcasterCharacterBP::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AcasterCharacterBP::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AcasterCharacterBP::TurnAtRate(float Value)
{
	AddControllerPitchInput(Value * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AcasterCharacterBP::LookUpAtRate(float Value)
{
	AddControllerPitchInput(Value * BaseLookUpAtRate * GetWorld()->GetDeltaSeconds());
}



// Called to bind functionality to input
void AcasterCharacterBP::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);


	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AcasterCharacterBP::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AcasterCharacterBP::LookUpAtRate);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AcasterCharacterBP::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AcasterCharacterBP::MoveRight);
	
	// Shoot
	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &AcasterCharacterBP::shoot);
}

float AcasterCharacterBP::getPlayerHP() {
	return playerHPpercent;
}


void AcasterCharacterBP::updatePlayerHP(float HP) {
	currentPlayerHP += HP;
	currentPlayerHP = FMath::Clamp(currentPlayerHP, 0.0f, maxPlayerHP);
	tempPlayerHP = playerHPpercent;
	playerHPpercent = currentPlayerHP / maxPlayerHP;
	if (playerHPpercent <= 0) {
		playerIsDead = true;
	}
	UE_LOG(LogTemp, Warning, TEXT("hp should update Alien"));
}

void AcasterCharacterBP::playerTakeDamage(float damage) {
	updatePlayerHP(-damage);
}

void AcasterCharacterBP::onRep_currentPlayerHP() {
	updatePlayerHP(0);
}

void AcasterCharacterBP::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AcasterCharacterBP, killerAlien);
	DOREPLIFETIME(AcasterCharacterBP, winnerPl);
	DOREPLIFETIME(AcasterCharacterBP, currentPlayerHP);
}

void AcasterCharacterBP::onRep_kill() {
	if (IsLocallyControlled()) {
		displayDeathScreen();
	}
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Block);
	SetLifeSpan(05.0f);
}

void AcasterCharacterBP::onRep_win() {
	if (IsLocallyControlled() && playerIsDead == false) {
		displayVictoryScreen();
	}
}

void AcasterCharacterBP::serverOnShoot_Implementation() {
	shoot();
}

void AcasterCharacterBP::shoot() {
	if (!HasAuthority()) {
		serverOnShoot();
	}
	else {
		// Get the camera transform.
		FVector CameraLocation;
		FRotator CameraRotation;
		GetActorEyesViewPoint(CameraLocation, CameraRotation);

		// Set MuzzleOffset to spawn projectiles slightly in front of the camera.
		MuzzleOffset.Set(80.0f, 0.0f, 0.0f);

		// Transform MuzzleOffset from camera space to world space.
		FVector MuzzleLocation = CameraLocation + FTransform(CameraRotation).TransformVector(MuzzleOffset);

		// Skew the aim to be slightly upwards.
		FRotator MuzzleRotation = CameraRotation;
		MuzzleRotation.Pitch += 0.0f;

		UWorld* World = GetWorld();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		Abullet* bullet = World->SpawnActor<Abullet>(BPbullet, MuzzleLocation, MuzzleRotation, SpawnParams);

		if (bullet) {
			FVector LaunchDirection = MuzzleRotation.Vector();
			bullet->FireInDirection(LaunchDirection);
		}
	}
}