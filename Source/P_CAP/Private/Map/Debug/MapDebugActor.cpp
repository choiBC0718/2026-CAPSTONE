#include "MapDebugActor.h"
#include "DrawDebugHelpers.h"
#include "Map/MapGenerator.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"

AMapDebugActor::AMapDebugActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMapDebugActor::BeginPlay()
{
	Super::BeginPlay();

	MapGenerator = NewObject<UMapGenerator>(this);

	if (bUseRandomSeedOnBeginPlay)
	{
		CurrentSeed = FMath::Rand();
	}

	BindDebugInput();
	GenerateAndDebug();
}

void AMapDebugActor::BindDebugInput()
{
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC) return;

	EnableInput(PC);

	if (InputComponent)
	{
		// R : 새 랜덤 시드로 생성
		InputComponent->BindKey(EKeys::R, IE_Pressed, this, &AMapDebugActor::RegenerateWithRandomSeed);

		// T : 현재 시드로 다시 생성
		InputComponent->BindKey(EKeys::T, IE_Pressed, this, &AMapDebugActor::RegenerateWithCurrentSeed);
	}
}

void AMapDebugActor::RegenerateWithRandomSeed()
{
	CurrentSeed = FMath::Rand();
	GenerateAndDebug();
}

void AMapDebugActor::RegenerateWithCurrentSeed()
{
	GenerateAndDebug();
}

void AMapDebugActor::GenerateAndDebug()
{
	if (!MapGenerator) return;

	FlushPersistentDebugLines(GetWorld());

	FMapGenerationConfig Config;
	Config.RoomCount = CurrentRoomCount;
	Config.Seed = CurrentSeed;

	FMapLayout Layout = MapGenerator->GenerateMap(Config);

	DebugDrawMap(Layout);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			1,
			5.0f,
			FColor::Yellow,
			FString::Printf(
				TEXT("Current Seed: %d   [R: Random Seed] [T: Rebuild Same Seed]"),
				CurrentSeed
			)
		);
	}
}

void AMapDebugActor::DebugDrawMap(const FMapLayout& Layout)
{
	const float TileSize = 300.f;

	for (const TPair<FIntPoint, FRoomData>& Pair : Layout.Rooms)
	{
		const FRoomData& Room = Pair.Value;

		FVector WorldPos = FVector(Room.GridPos.X * TileSize, Room.GridPos.Y * TileSize, 0.f);

		FColor Color = FColor::White;

		if (Room.RoomType == ERoomType::Start)
		{
			Color = FColor::Green;
		}
		else if (Room.RoomType == ERoomType::Boss)
		{
			Color = FColor::Red;
		}

		DrawDebugBox(GetWorld(), WorldPos, FVector(100, 100, 50), Color, true);
	}
}