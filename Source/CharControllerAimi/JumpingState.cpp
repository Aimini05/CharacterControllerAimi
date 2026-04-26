// Fill out your copyright notice in the Description page of Project Settings.


#include "JumpingState.h"
#include "CustomPlayerPawn.h"
#include "Engine/Engine.h"

void FJumpingState::Enter(ACustomPlayerPawn* Pawn)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("Entered Jumping"));
	}
}

void FJumpingState::Update(ACustomPlayerPawn* Pawn, float DeltaTime)
{
}

void FJumpingState::Exit(ACustomPlayerPawn* Pawn)
{
}