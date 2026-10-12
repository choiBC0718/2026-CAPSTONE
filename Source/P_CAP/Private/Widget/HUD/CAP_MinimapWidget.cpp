#include "Widget/HUD/CAP_MinimapWidget.h"

#include "Rendering/DrawElements.h"
#include "Kismet/GameplayStatics.h"
#include "Map/Debug/MapManager.h"

namespace
{
	constexpr float MapPanelSize = 248.f;
	constexpr float MaxRoomTileSize = 22.f;
	constexpr float MapPadding = 14.f;
	constexpr float HeaderHeight = 34.f;
	constexpr float LegendHeight = 38.f;

	FLinearColor GetRoomColor(ERoomType RoomType)
	{
		switch (RoomType)
		{
		case ERoomType::Start: return FLinearColor(0.1f, 0.75f, 0.25f, 1.f);
		case ERoomType::Boss: return FLinearColor(0.85f, 0.12f, 0.12f, 1.f);
		case ERoomType::Reward: return FLinearColor(0.95f, 0.75f, 0.1f, 1.f);
		case ERoomType::Shop: return FLinearColor(0.15f, 0.4f, 0.95f, 1.f);
		case ERoomType::Event: return FLinearColor(0.8f, 0.8f, 0.85f, 1.f);
		default: return FLinearColor(0.45f, 0.48f, 0.52f, 1.f);
		}
	}
}

void UCAP_MinimapWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetAlignmentInViewport(FVector2D(1.f, 0.f));
	FVector2D ViewportSize = FVector2D::ZeroVector;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	SetPositionInViewport(FVector2D(ViewportSize.X - 24.f, 24.f));
	SetDesiredSizeInViewport(FVector2D(MapPanelSize));
	if (AMapManager* MapManager = Cast<AMapManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AMapManager::StaticClass())))
	{
		SetMapLayout(MapManager->GetCurrentLayout());
		if (MapManager->HasRestoredSpecialRoomState())
		{
			RestoreVisitedRooms(MapManager->GetRestoredVisitedRoomGridPositions());
			SetCurrentRoom(MapManager->GetRestoredCurrentRoomGridPos());
		}
		else
		{
			SetCurrentRoom(MapManager->GetCurrentLayout().StartRoomPos);
		}
	}
}

void UCAP_MinimapWidget::SetMapLayout(const FMapLayout& InLayout)
{
	MapLayout = InLayout;
	VisitedRoomGridPositions.Reset();
	bHasCurrentRoom = false;
	Invalidate(EInvalidateWidgetReason::Paint);
}

void UCAP_MinimapWidget::SetCurrentRoom(const FIntPoint& InGridPos)
{
	CurrentRoomGridPos = InGridPos;
	bHasCurrentRoom = true;
	VisitedRoomGridPositions.Add(InGridPos);
	Invalidate(EInvalidateWidgetReason::Paint);
}

void UCAP_MinimapWidget::RestoreVisitedRooms(const TSet<FIntPoint>& InVisitedRoomGridPositions)
{
	VisitedRoomGridPositions = InVisitedRoomGridPositions;
	Invalidate(EInvalidateWidgetReason::Paint);
}

int32 UCAP_MinimapWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FVector2D PanelTopLeft(0.f, 0.f);
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
	const FVector2D PanelSize(MapPanelSize, MapPanelSize);
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(
		PanelSize, FSlateLayoutTransform(PanelTopLeft)), WhiteBrush, ESlateDrawEffect::None,
		FLinearColor(0.16f, 0.19f, 0.24f, 0.94f));
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(
		PanelSize - FVector2D(2.f), FSlateLayoutTransform(FVector2D(1.f))), WhiteBrush,
		ESlateDrawEffect::None, FLinearColor(0.025f, 0.035f, 0.05f, 0.92f));

	const FSlateFontInfo HeaderFont = FCoreStyle::GetDefaultFontStyle("Bold", 12);
	FSlateDrawElement::MakeText(OutDrawElements, LayerId + 2,
		AllottedGeometry.ToPaintGeometry(FVector2D(60.f, 18.f), FSlateLayoutTransform(FVector2D(14.f, 8.f))),
		FText::FromString(TEXT("MAP")), HeaderFont, ESlateDrawEffect::None,
		FLinearColor(0.9f, 0.93f, 0.98f, 1.f));
	TArray<FVector2D> HeaderDivider = { FVector2D(14.f, HeaderHeight - 1.f), FVector2D(MapPanelSize - 14.f, HeaderHeight - 1.f) };
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 2, AllottedGeometry.ToPaintGeometry(),
		HeaderDivider, ESlateDrawEffect::None, FLinearColor(0.38f, 0.43f, 0.5f, 0.65f), true, 1.f);

	if (MapLayout.Rooms.IsEmpty())
	{
		return LayerId + 3;
	}

	int32 MinX = MAX_int32, MaxX = MIN_int32, MinY = MAX_int32, MaxY = MIN_int32;
	for (const TPair<FIntPoint, FRoomData>& Pair : MapLayout.Rooms)
	{
		MinX = FMath::Min(MinX, Pair.Key.X); MaxX = FMath::Max(MaxX, Pair.Key.X);
		MinY = FMath::Min(MinY, Pair.Key.Y); MaxY = FMath::Max(MaxY, Pair.Key.Y);
	}
	// The fixed gameplay camera looks toward +X: world Y runs left-to-right,
	// while increasing world X moves upward on screen. Scale the grid to fit above the legend.
	const int32 ColumnCount = MaxY - MinY + 1;
	const int32 RowCount = MaxX - MinX + 1;
	const FVector2D MapAreaTopLeft(MapPadding, HeaderHeight + 4.f);
	const FVector2D MapAreaSize(MapPanelSize - MapPadding * 2.f,
		MapPanelSize - HeaderHeight - LegendHeight - MapPadding - 4.f);
	const float Step = FMath::Min(MapAreaSize.X / ColumnCount, MapAreaSize.Y / RowCount);
	const float RoomTileSize = FMath::Min(MaxRoomTileSize, FMath::Max(12.f, Step - 5.f));
	const FVector2D GridExtent((ColumnCount - 1) * Step + RoomTileSize,
		(RowCount - 1) * Step + RoomTileSize);
	const FVector2D ContentOrigin = MapAreaTopLeft + (MapAreaSize - GridExtent) * 0.5f;

	auto RoomCenter = [&](const FIntPoint& Pos)
	{
		return ContentOrigin + FVector2D((Pos.Y - MinY) * Step + RoomTileSize * 0.5f,
			(MaxX - Pos.X) * Step + RoomTileSize * 0.5f);
	};

	// Draw passages first, so room tiles sit cleanly over their ends.
	for (const TPair<FIntPoint, FRoomData>& Pair : MapLayout.Rooms)
	{
		const FRoomData& Room = Pair.Value;
		const FVector2D Center = RoomCenter(Room.GridPos);
		const FVector2D Half(RoomTileSize * 0.5f, RoomTileSize * 0.5f);
		const FVector2D Directions[] = { FVector2D(1, 0), FVector2D(-1, 0), FVector2D(0, 1), FVector2D(0, -1) };
		const bool Connected[] = { Room.bConnectedUp, Room.bConnectedDown, Room.bConnectedLeft, Room.bConnectedRight };
		for (int32 Index = 0; Index < 4; ++Index)
		{
			if (!Connected[Index]) continue;
			const FVector2D Start = Center + Directions[Index] * Half;
			const FVector2D End = Center + Directions[Index] * (Half + FVector2D(Step - RoomTileSize, Step - RoomTileSize));
			TArray<FVector2D> Points = { Start, End };
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 3,
				AllottedGeometry.ToPaintGeometry(), Points, ESlateDrawEffect::None,
				FLinearColor(0.48f, 0.55f, 0.63f, 0.62f), true, 1.5f);
		}
	}

	for (const TPair<FIntPoint, FRoomData>& Pair : MapLayout.Rooms)
	{
		const FRoomData& Room = Pair.Value;
		const FVector2D TopLeft = RoomCenter(Room.GridPos) - FVector2D(RoomTileSize * 0.5f);
		FLinearColor Color = GetRoomColor(Room.RoomType);
		const bool bIsCurrentRoom = bHasCurrentRoom && CurrentRoomGridPos == Room.GridPos;
		if (!VisitedRoomGridPositions.Contains(Room.GridPos) && !bIsCurrentRoom)
		{
			Color.R *= 0.32f;
			Color.G *= 0.32f;
			Color.B *= 0.32f;
		}
		if (bIsCurrentRoom)
		{
			const FVector2D GlowSize(RoomTileSize + 10.f);
			FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 4,
				AllottedGeometry.ToPaintGeometry(GlowSize, FSlateLayoutTransform(TopLeft - FVector2D(5.f))),
				WhiteBrush, ESlateDrawEffect::None, FLinearColor(1.f, 0.68f, 0.08f, 0.22f));
			const FVector2D BorderSize(RoomTileSize + 6.f);
			FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 5,
				AllottedGeometry.ToPaintGeometry(BorderSize, FSlateLayoutTransform(TopLeft - FVector2D(3.f))),
				WhiteBrush, ESlateDrawEffect::None, FLinearColor(1.f, 0.75f, 0.12f, 1.f));
		}
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 6,
			AllottedGeometry.ToPaintGeometry(FVector2D(RoomTileSize), FSlateLayoutTransform(TopLeft)),
			WhiteBrush, ESlateDrawEffect::None, Color);
	}
	const FSlateFontInfo LegendFont = FCoreStyle::GetDefaultFontStyle("Regular", 8);
	auto DrawLegendItem = [&](const TCHAR* Label, ERoomType RoomType, const FVector2D& Position)
	{
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 7,
			AllottedGeometry.ToPaintGeometry(FVector2D(8.f), FSlateLayoutTransform(Position)),
			WhiteBrush, ESlateDrawEffect::None, GetRoomColor(RoomType));
		FSlateDrawElement::MakeText(OutDrawElements, LayerId + 7,
			AllottedGeometry.ToPaintGeometry(FVector2D(68.f, 12.f), FSlateLayoutTransform(Position + FVector2D(12.f, -2.f))),
			FText::FromString(Label), LegendFont, ESlateDrawEffect::None,
			FLinearColor(0.76f, 0.8f, 0.86f, 0.95f));
	};
	const float LegendTop = MapPanelSize - LegendHeight + 4.f;
	DrawLegendItem(TEXT("START"), ERoomType::Start, FVector2D(14.f, LegendTop));
	DrawLegendItem(TEXT("SHOP"), ERoomType::Shop, FVector2D(128.f, LegendTop));
	DrawLegendItem(TEXT("REWARD"), ERoomType::Reward, FVector2D(14.f, LegendTop + 15.f));
	DrawLegendItem(TEXT("BOSS"), ERoomType::Boss, FVector2D(128.f, LegendTop + 15.f));
	return LayerId + 8;
}
