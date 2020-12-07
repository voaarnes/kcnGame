// Fill out your copyright notice in the Description page of Project Settings.


#include "ffaGameMode.h"
#include "ffaPlayerController.h"
#include "SpellshooterPlayerController.h"
#include "ffaPlayerState.h"
#include "ffaGameState.h"
#include "humanCharacter.h"
#include "casterCharacterBP.h"

AffaGameMode::AffaGameMode()
	: Super() {
}

void AffaGameMode::PostLogin(APlayerController* newPlayer)
{
	Super::PostLogin(newPlayer);

	//playersAlive.Add(Cast<AffaPlayerController>(newPlayer));
	playersAlive.Add(Cast<ASpellshooterPlayerController>(newPlayer));
}

void AffaGameMode::playerKilled(class AHumanCharacter* killed, class AHumanCharacter* killer, class AcasterCharacterBP* alienKilled, class AcasterCharacterBP* alienKiller) {

	if (killed) {//&& killed != nullptr) {
		//if (AffaPlayerController* player = Cast<AffaPlayerController>(killed->GetController())) {
		if (ASpellshooterPlayerController* player = Cast<ASpellshooterPlayerController>(killed->GetController())) {

			playersAlive.RemoveSingle(player);
		}
		if (playersAlive.Num() == 1 && playersAlive.IsValidIndex(0)) {
			winPlayer(Cast<AffaPlayerState>(playersAlive[0]->PlayerState));
		}
	}
	/*else if (alienKilled && alienKilled != nullptr) {
		if (AffaPlayerController* player = Cast<AffaPlayerController>(killed->GetController())) {
			playersAlive.RemoveSingle(player);
		}
		if (playersAlive.Num() == 1 && playersAlive.IsValidIndex(0)) {
			winPlayer(Cast<AffaPlayerState>(playersAlive[0]->PlayerState));
		}
	}*/
}

void AffaGameMode::winPlayer(class AffaPlayerState* winner)
{
	if (AffaGameState* state = GetGameState<AffaGameState>()) {
		state->winner = winner;
	}
}
