#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Map/MapLayout.h"
#include "CAP_MinimapWidget.generated.h"

/** Small schematic map drawn from the generated room layout. */
UCLASS()
class UCAP_MinimapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void SetMapLayout(const FMapLayout& InLayout);
	void SetCurrentRoom(const FIntPoint& InGridPos);
	void RestoreVisitedRooms(const TSet<FIntPoint>& InVisitedRoomGridPositions);
	const TSet<FIntPoint>& GetVisitedRoomGridPositions() const { return VisitedRoomGridPositions; }

protected:
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	FMapLayout MapLayout;
	FIntPoint CurrentRoomGridPos = FIntPoint::ZeroValue;
	TSet<FIntPoint> VisitedRoomGridPositions;
	bool bHasCurrentRoom = false;
};
