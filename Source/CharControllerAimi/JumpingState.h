// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PlayerMovementStateBase.h"

class FJumpingState : public FPlayerMovementStateBase
{
public:
	virtual void Enter(ACustomPlayerPawn* Pawn) override;
	virtual void Update(ACustomPlayerPawn* Pawn, float DeltaTime) override;
	virtual void Exit(ACustomPlayerPawn* Pawn) override;
	virtual const TCHAR* GetName() const override { return TEXT("Jumping"); }
};