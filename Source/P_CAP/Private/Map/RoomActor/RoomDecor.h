// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class ARoomActor;
struct FRoomInteriorLayout;

class FRoomDecor
{
public:
	static void Clear(ARoomActor& Room);
	static void SpawnDecorations(ARoomActor& Room, FRoomInteriorLayout& Layout);
};
