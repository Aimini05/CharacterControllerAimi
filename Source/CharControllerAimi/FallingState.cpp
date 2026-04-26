// Fill out your copyright notice in the Description page of Project Settings.


#include "FallingState.h"
#include "GroundedState.h"
#include "CustomPlayerPawn.h"
#include "Engine/Engine.h"

void FFallingState::Enter(ACustomPlayerPawn* Pawn)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("Entered Falling"));
	}
}

void FFallingState::Update(ACustomPlayerPawn* Pawn, float DeltaTime)
{
	if (!Pawn) return;

	if (Pawn->IsGrounded())
	{
		Pawn->ChangeState(MakeUnique<FGroundedState>());
	}
}

void FFallingState::Exit(ACustomPlayerPawn* Pawn)
{
}