// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class ARoomActor;

class FRoomTemplate
{
public:
	static void Clear(ARoomActor& Room);
	static void Select(ARoomActor& Room);
	static void Spawn(ARoomActor& Room);
};
