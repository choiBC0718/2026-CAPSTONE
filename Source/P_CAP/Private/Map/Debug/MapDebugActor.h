#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Map/MapGenerator.h"
#include "MapDebugActor.generated.h"

UCLASS()
class AMapDebugActor : public AActor
{
	GENERATED_BODY()
	
public:
	AMapDebugActor();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UMapGenerator* MapGenerator;

	// 에디터에서 직접 입력할 시드
	UPROPERTY(EditAnywhere, Category="Map Debug")
	int32 CurrentSeed = 12345;

	// 방 개수
	UPROPERTY(EditAnywhere, Category="Map Debug")
	int32 CurrentRoomCount = 10;

	// 시작 시 랜덤 시드 사용할지
	UPROPERTY(EditAnywhere, Category="Map Debug")
	bool bUseRandomSeedOnBeginPlay = false;

	void GenerateAndDebug();
	void DebugDrawMap(const FMapLayout& Layout);

	void BindDebugInput();
	void RegenerateWithRandomSeed();
	void RegenerateWithCurrentSeed();
};