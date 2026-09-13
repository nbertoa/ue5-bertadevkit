#include "BertaWindowGeometry.h"

namespace UE::BertaWindowTools::Private
{
	namespace
	{
		int64 CalculateIntersectionArea(const FPlatformRect& A, const FPlatformRect& B)
		{
			const int64 Width = FMath::Max<int64>(0, FMath::Min<int64>(A.Right, B.Right) - FMath::Max<int64>(A.Left, B.Left));
			const int64 Height = FMath::Max<int64>(0, FMath::Min<int64>(A.Bottom, B.Bottom) - FMath::Max<int64>(A.Top, B.Top));
			return Width * Height;
		}

		double CalculateSquaredDistanceToRect(double X, double Y, const FPlatformRect& Rect)
		{
			const double ClosestX = FMath::Clamp(X, static_cast<double>(Rect.Left), static_cast<double>(Rect.Right));
			const double ClosestY = FMath::Clamp(Y, static_cast<double>(Rect.Top), static_cast<double>(Rect.Bottom));
			const double DeltaX = X - ClosestX;
			const double DeltaY = Y - ClosestY;
			return DeltaX * DeltaX + DeltaY * DeltaY;
		}
	}

	FIntPoint CalculateCenteredPosition(const FPlatformRect& Area, const FIntPoint WindowSize)
	{
		const int64 AreaWidth = static_cast<int64>(Area.Right) - Area.Left;
		const int64 AreaHeight = static_cast<int64>(Area.Bottom) - Area.Top;
		if (WindowSize.X > AreaWidth || WindowSize.Y > AreaHeight)
		{
			return FIntPoint(Area.Left, Area.Top);
		}

		const int64 X = static_cast<int64>(Area.Left) + (AreaWidth - WindowSize.X) / 2;
		const int64 Y = static_cast<int64>(Area.Top) + (AreaHeight - WindowSize.Y) / 2;
		return FIntPoint(static_cast<int32>(X), static_cast<int32>(Y));
	}

	int32 FindBestMonitorIndex(const TArray<FMonitorInfo>& Monitors, const FPlatformRect& WindowRect)
	{
		int32 BestIndex = INDEX_NONE;
		int64 BestIntersectionArea = 0;
		for (int32 Index = 0; Index < Monitors.Num(); ++Index)
		{
			const int64 IntersectionArea = CalculateIntersectionArea(WindowRect, Monitors[Index].DisplayRect);
			if (IntersectionArea > BestIntersectionArea)
			{
				BestIntersectionArea = IntersectionArea;
				BestIndex = Index;
			}
		}

		if (BestIntersectionArea > 0)
		{
			return BestIndex;
		}

		const double WindowCenterX = (static_cast<double>(WindowRect.Left) + WindowRect.Right) * 0.5;
		const double WindowCenterY = (static_cast<double>(WindowRect.Top) + WindowRect.Bottom) * 0.5;
		double BestSquaredDistance = TNumericLimits<double>::Max();
		for (int32 Index = 0; Index < Monitors.Num(); ++Index)
		{
			const double SquaredDistance = CalculateSquaredDistanceToRect(WindowCenterX, WindowCenterY, Monitors[Index].DisplayRect);
			if (SquaredDistance < BestSquaredDistance)
			{
				BestSquaredDistance = SquaredDistance;
				BestIndex = Index;
			}
		}

		return BestIndex;
	}
}
