// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class ACustomPlayerPawn;

class FPlayerMovementStateBase
{
public:
	virtual ~FPlayerMovementStateBase() = default;
	
	virtual void Enter(ACustomPlayerPawn* Pawn) {}
	virtual void Exit(ACustomPlayerPawn* Pawn) {}
	virtual void Update(ACustomPlayerPawn* Pawn, float DeltaTime) {}
	virtual const TCHAR* GetName() const = 0;
};
