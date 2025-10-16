// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UObject/NoExportTypes.h"

#include "MetaXRAudioSpectrum.generated.h"

#define META_XR_AUDIO_MIN_FREQUENCY 0.0f
#define META_XR_AUDIO_MAX_FREQUENCY 20000.0f
#define META_XR_AUDIO_MIN_NUM_SPECTRUM_POINTS 0
#define META_XR_AUDIO_MAX_NUM_SPECTRUM_POINTS 10

USTRUCT(BlueprintType)
struct FMetaXRAudioPoint {
  GENERATED_BODY()

  UPROPERTY()
  float Frequency;

  UPROPERTY()
  float Data;

  // #if WITH_EDITORONLY_DATA
  FMetaXRAudioPoint(float InFrequency = 0.0f, float InData = 0.0f) : Frequency(InFrequency), Data(InData) {}

  bool operator==(const FMetaXRAudioPoint& Other) const {
    return (Other.Frequency == Frequency) && (Other.Data == Data);
  }

  static FMetaXRAudioPoint FromVector2D(const FVector2D& Vector) {
    return FMetaXRAudioPoint(Vector.X, Vector.Y);
  }

  operator FVector2D() const {
    return FVector2D(Frequency, Data);
  }
  // #endif
};

USTRUCT(BlueprintType)
struct FMetaXRAudioSpectrum {
  GENERATED_BODY()

  int32 Selection = MAX_int32;
  bool IsDirty = false;

  UPROPERTY()
  TArray<FMetaXRAudioPoint> Points;

  void Clone(const FMetaXRAudioSpectrum& Other) {
    if (this == &Other) {
      return;
    }

    Selection = Other.Selection;
    Points = TArray<FMetaXRAudioPoint>(Other.Points);
  }

  void Sort() {
    if (Points.Num() > 0) {
      Points.Sort([](const FMetaXRAudioPoint& A, const FMetaXRAudioPoint& B) { return A.Frequency < B.Frequency; });
      if (Selection != MAX_int32) {
        FMetaXRAudioPoint SelectedPoint = Points[Selection];
        Selection = Points.IndexOfByKey(SelectedPoint);
      }
    }
  }

  float operator[](float Frequency) const {
    if (Points.Num() > 0) {
      FMetaXRAudioPoint Lower = FMetaXRAudioPoint(FLT_MIN);
      FMetaXRAudioPoint Upper = FMetaXRAudioPoint(FLT_MAX);

      for (const FMetaXRAudioPoint& Point : Points) {
        if (Point.Frequency < Frequency) {
          if (Point.Frequency > Lower.Frequency)
            Lower = Point;
        } else {
          if (Point.Frequency < Upper.Frequency)
            Upper = Point;
        }
      }

      if (Lower.Frequency == FLT_MIN) {
        Lower.Data = Points[0].Data;
      }

      if (Upper.Frequency == FLT_MAX) {
        Upper.Data = Points.Last().Data;
      }

      return FMath::Lerp(Lower.Data, Upper.Data, (Frequency - Lower.Frequency) / (Upper.Frequency - Lower.Frequency));
    }

    return 0.0f;
  }
};
