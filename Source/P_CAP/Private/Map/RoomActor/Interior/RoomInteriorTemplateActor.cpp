// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/Interior/RoomInteriorTemplateActor.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"

ARoomInteriorTemplateActor::ARoomInteriorTemplateActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void ARoomInteriorTemplateActor::InitializeTemplate(int32 InMapSeed, const FIntPoint& InRoomGridPos)
{
	MapSeed = InMapSeed;
	RoomGridPos = InRoomGridPos;

	OnTemplateInitialized(MapSeed, RoomGridPos);
}

void ARoomInteriorTemplateActor::GetAllFloorExclusionBoxes(TArray<FRoomInteriorFloorExclusionBox>& OutBoxes) const
{
	OutBoxes.Append(FloorExclusionBoxes);

	TSet<const UBoxComponent*> AddedComponents;
	auto AddBoxComponent = [&OutBoxes, &AddedComponents](const UBoxComponent* BoxComponent)
	{
		if (!BoxComponent || AddedComponents.Contains(BoxComponent))
		{
			return;
		}

		AddedComponents.Add(BoxComponent);

		FTransform ComponentToActor = BoxComponent->GetRelativeTransform();
		for (const USceneComponent* Parent = BoxComponent->GetAttachParent(); Parent; Parent = Parent->GetAttachParent())
		{
			ComponentToActor *= Parent->GetRelativeTransform();
		}

		const FVector ComponentScale = ComponentToActor.GetScale3D().GetAbs();
		FRoomInteriorFloorExclusionBox& NewBox = OutBoxes.AddDefaulted_GetRef();
		NewBox.Center = ComponentToActor.GetLocation();
		NewBox.Extent = BoxComponent->GetUnscaledBoxExtent() * ComponentScale;
	};

	for (const UBoxComponent* BoxComponent : FloorExclusionBoxComponents)
	{
		AddBoxComponent(BoxComponent);
	}

	TArray<UBoxComponent*> BoxComponents;
	GetComponents<UBoxComponent>(BoxComponents);
	for (const UBoxComponent* BoxComponent : BoxComponents)
	{
		const bool bMatchesTag = FloorExclusionComponentTag != NAME_None && BoxComponent->ComponentHasTag(FloorExclusionComponentTag);
		const bool bMatchesName = !FloorExclusionComponentNamePrefix.IsEmpty() && BoxComponent->GetName().StartsWith(FloorExclusionComponentNamePrefix);
		if (bMatchesTag || bMatchesName)
		{
			AddBoxComponent(BoxComponent);
		}
	}
}
