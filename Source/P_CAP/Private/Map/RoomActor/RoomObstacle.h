// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

struct FPlayerTendencyModifier;
class ARoomActor;

class FRoomObstacle
{
public:
	static void Clear(ARoomActor& Room);
	static void SpawnByTendency(ARoomActor& Room, const FPlayerTendencyModifier& Tendency);
};
