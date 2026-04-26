// Fill out your copyright notice in the Description page of Project Settings.


#include "GroundedState.h"
#include "FallingState.h"
#include "JumpingState.h"
#include "CustomPlayerPawn.h"
#include "Engine/Engine.h"

void FGroundedState::Enter(ACustomPlayerPawn* Pawn)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Entered Grounded"));
	}
}

void FGroundedState::Update(ACustomPlayerPawn* Pawn, float DeltaTime)
{
	if (!Pawn) return;

	if (!Pawn->IsGrounded())
	{
		Pawn->ChangeState(MakeUnique<FFallingState>());
		return;
	}

	// Jump transition kan vi lägga till strax efter
}

void FGroundedState::Exit(ACustomPlayerPawn* Pawn)
{
}