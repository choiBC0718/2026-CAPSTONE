// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class ARoomActor;
struct FRoomInteriorLayout;

class FRoomStructure
{
public:
	static void Clear(ARoomActor& Room);
	static void SpawnLargeMeshes(ARoomActor& Room, const FRoomInteriorLayout& Layout);
	static void DrawDebugCells(const ARoomActor& Room, const FRoomInteriorLayout& Layout);
};
