// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "casterCharacterBP.generated.h"


class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent; //might be obsolete


UCLASS()
class SPELLSHOOTER_API AcasterCharacterBP : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AcasterCharacterBP();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		USpringArmComponent* SpringArmComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		UCameraComponent* CameraComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")   //might be obsolete
		USkeletalMeshComponent* MeshComp;

	// Player health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "health")
		float maxPlayerHP;

	UPROPERTY(ReplicatedUsing = onRep_currentPlayerHP, EditAnywhere, BlueprintReadWrite, Category = "health") //-------->
		float currentPlayerHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "health")
		float tempPlayerHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "health")
		float playerHPpercent;

	UFUNCTION(BlueprintPure, Category = "health")
		float getPlayerHP();

	UFUNCTION(BlueprintCallable, Category = "health")
		void updatePlayerHP(float HP);


	UFUNCTION(BlueprintCallable, Category = "health")
		void playerTakeDamage(float damage);

	UPROPERTY(ReplicatedUsing = onRep_Kill, BlueprintReadOnly, Category = Gameplay)//---------
		AcasterCharacterBP* killer;

	UFUNCTION()
		void onRep_kill();

	UFUNCTION(BlueprintImplementableEvent)
		void displayDeathScreen();

	UFUNCTION()
		void onRep_currentPlayerHP();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Value);
	void LookUpAtRate(float Value);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		float BaseLookUpAtRate;
		
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
