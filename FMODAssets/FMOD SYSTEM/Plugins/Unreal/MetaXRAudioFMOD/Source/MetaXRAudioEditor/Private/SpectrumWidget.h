// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#include "MetaXRAudioSpectrum.h"

UENUM(BlueprintType)
enum class EAxisScale : uint8 {
  Linear UMETA(DisplayName = "Linear"),
  Log UMETA(DisplayName = "Log"),
  Square UMETA(DisplayName = "Square"),
  Cube UMETA(DisplayName = "Cube"),
  SquareCentered UMETA(DisplayName = "SquareCentered")
};

class SSpectrumWidget : public SCompoundWidget {
 public:
  SLATE_BEGIN_ARGS(SSpectrumWidget) {}
  SLATE_ATTRIBUTE(FMetaXRAudioSpectrum*, Spectrum) // Add a Slate attribute for the spectrum
  SLATE_ATTRIBUTE(FLinearColor, GraphColor) // Add a Slate attribute for the graph color
  SLATE_ATTRIBUTE(EAxisScale, Scale) // Add a Slate attribute for the axis scale
  SLATE_ATTRIBUTE(float, RangeMin) // Add a Slate attribute for the range min
  SLATE_ATTRIBUTE(float, RangeMax) // Add a Slate attribute for the range max
  SLATE_END_ARGS()

  void Construct(const FArguments& InArgs);
  void SetSpectrum(FMetaXRAudioSpectrum* Spectrum) {
    MySpectrum = Spectrum;
  }

 protected:
  virtual int32 OnPaint(
      const FPaintArgs& Args,
      const FGeometry& AllottedGeometry,
      const FSlateRect& MyCullingRect,
      FSlateWindowElementList& OutDrawElements,
      int32 LayerId,
      const FWidgetStyle& InWidgetStyle,
      bool bParentEnabled) const override;
  virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
  virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
  FMetaXRAudioPoint MapMouseEvent(const FGeometry& Geometry, const FVector2D& MousePosition);
  virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

  static bool MapFrequencyTick(int32 i, float& Frequency);
  static FString FrequencyToString(float Frequency);
  static FString DataToString(float Data);
  static FString FrequencyToTickString(float Frequency);

 private:
  void DrawMinMaxFilledCurve(
      const FPaintArgs& Args,
      const FGeometry& AllottedGeometry,
      const FSlateRect& MyCullingRect,
      FSlateWindowElementList& OutDrawElements,
      int32 LayerId,
      const FWidgetStyle& InWidgetStyle,
      bool bParentEnabled,
      const FMetaXRAudioSpectrum& Spectrum) const;
  void DrawCurve(
      const FPaintArgs& Args,
      const FGeometry& AllottedGeometry,
      int32 LayerId,
      FSlateWindowElementList& OutDrawElements,
      FLinearColor CurveColor) const;
  void DrawBorder(const FPaintArgs& Args, const FGeometry& AllottedGeometry, int32 LayerId, FSlateWindowElementList& OutDrawElements) const;
  void DrawDataTicks(
      const FPaintArgs& Args,
      const FGeometry& AllottedGeometry,
      const FSlateRect& MyCullingRect,
      FSlateWindowElementList& OutDrawElements,
      int32 LayerId,
      const FWidgetStyle& InWidgetStyle) const;
  void DrawFrequencyTicks(
      const FPaintArgs& Args,
      const FGeometry& AllottedGeometry,
      const FSlateRect& MyCullingRect,
      FSlateWindowElementList& OutDrawElements,
      int32 LayerId,
      const FWidgetStyle& InWidgetStyle) const;
  void EvaluateCurveMinMaxColor(
      const FMetaXRAudioSpectrum& Spectrum,
      float Frequency,
      FLinearColor& OutColor,
      float& OutMinValue,
      float& OutMaxValue) const;
  float EvaluateCurve(const FMetaXRAudioSpectrum& Spectrum, float Frequency) const;
  static float MapFrequency(float f, bool forward = true);
  float MapData(float Value, bool bForward = true) const;
  FVector2D MapPointPosition(const FGeometry& AllottedGeometry, const FMetaXRAudioPoint& Point) const;
  void DrawSelected(
      const FMetaXRAudioSpectrum& Spectrum,
      const FGeometry& AllottedGeometry,
      FSlateWindowElementList& OutDrawElements,
      int32 LayerId) const;
  FMetaXRAudioSpectrum* MySpectrum = nullptr;
  TAttribute<FLinearColor> GraphColor;
  float RangeOrigin = 0.0f;
  float RangeMin = 0.0f;
  float RangeMax = 1.0f;
  EAxisScale Scale = EAxisScale::Cube;

  inline static float FrequencyMax = 20000.0f;
};
