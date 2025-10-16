// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#include "SpectrumWidget.h"
#include "Brushes/SlateColorBrush.h"
#include "EditorStyleSet.h"

void SSpectrumWidget::Construct(const FArguments& InArgs) {
  MySpectrum = InArgs._Spectrum.Get();
  GraphColor = InArgs._GraphColor;
  Scale = InArgs._Scale.Get();
  RangeMin = InArgs._RangeMin.Get();
  RangeMax = InArgs._RangeMax.Get();
}

float SSpectrumWidget::MapFrequency(float f, bool forward) {
  if (forward) {
    return f < 10.0f ? 0.0f : FMath::Loge(f / 10.0f) / FMath::Loge(FrequencyMax / 10.0f);
  } else {
    return 10.0f * FMath::Pow(FrequencyMax / 10.0f, f);
  }
}

void SSpectrumWidget::EvaluateCurveMinMaxColor(
    const FMetaXRAudioSpectrum& Spectrum,
    float Frequency,
    FLinearColor& OutColor,
    float& OutMinValue,
    float& OutMaxValue) const {
  float y = 2.0f * MapData(Spectrum[MapFrequency(Frequency, false)]) - 1.0f;
  float c = 2.0f * (RangeOrigin - RangeMin) / (RangeMax - RangeMin) - 1.0f;
  OutMinValue = FMath::Min(y, c);
  OutMaxValue = FMath::Max(y, c);
  OutColor = GraphColor.Get();
}

float SSpectrumWidget::EvaluateCurve(const FMetaXRAudioSpectrum& Spectrum, float Frequency) const {
  return 2 * MapData(Spectrum[MapFrequency(Frequency, false)]) - 1;
}

void GetPointCache(int32 NumPoints, TArray<FVector2D>& OutPointCache) {
  OutPointCache.SetNumUninitialized(NumPoints);
}

void SSpectrumWidget::DrawMinMaxFilledCurve(
    const FPaintArgs& Args,
    const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect,
    FSlateWindowElementList& OutDrawElements,
    int32 LayerId,
    const FWidgetStyle& InWidgetStyle,
    bool bParentEnabled,
    const FMetaXRAudioSpectrum& Spectrum) const {
  TArray<FSlateVertex> Vertices;
  TArray<SlateIndex> Indices;

  FVector2D Size = AllottedGeometry.GetAbsoluteSize();
  if (Size.X <= 0 || Size.Y <= 0) {
    return;
  }

  const int32 NumPoints = FMath::CeilToInt(Size.X);
  const float HalfHeight = Size.Y * 0.5f;
  const float Step = 1.0f / (NumPoints - 1);

  Vertices.SetNumUninitialized(NumPoints * 2); // Two vertices per point

  FColor FinalColor = GraphColor.Get().ToFColor(false);
  FinalColor.A = 76;

  for (int32 i = 0; i < NumPoints; ++i) {
    const float Frequency = i * Step;

    // Vertex at the point's y-coordinate
    Vertices[i * 2].Position = {
        i + AllottedGeometry.AbsolutePosition.X,
        AllottedGeometry.AbsolutePosition.Y + HalfHeight - HalfHeight * EvaluateCurve(*MySpectrum, Frequency)};
    Vertices[i * 2].Color = FinalColor;

    // Vertex at the bottom of the geometry
    Vertices[i * 2 + 1].Position = {
        i + AllottedGeometry.AbsolutePosition.X, AllottedGeometry.AbsolutePosition.Y + static_cast<float>(Size.Y)};
    Vertices[i * 2 + 1].Color = FinalColor;
  }

  Indices.SetNumUninitialized((NumPoints - 1) * 6); // Three indices per triangle

  for (int32 i = 0; i < NumPoints - 1; ++i) {
    Indices[i * 6] = i * 2;
    Indices[i * 6 + 1] = i * 2 + 1;
    Indices[i * 6 + 2] = (i + 1) * 2;

    Indices[i * 6 + 3] = i * 2 + 1;
    Indices[i * 6 + 4] = (i + 1) * 2 + 1;
    Indices[i * 6 + 5] = (i + 1) * 2;
  }

  const FSlateColorBrush WhiteBrush(GraphColor.Get());
  const FSlateBrush* SlateBrush = &WhiteBrush;
  const FSlateResourceHandle Handle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*SlateBrush);

  FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, Handle, Vertices, Indices, nullptr, 0, 0);
}

void SSpectrumWidget::DrawCurve(
    const FPaintArgs& Args,
    const FGeometry& AllottedGeometry,
    int32 LayerId,
    FSlateWindowElementList& OutDrawElements,
    FLinearColor CurveColor) const {
  // Ensure that there are points to draw
  if (MySpectrum && MySpectrum->Points.Num() > 1) {
    TArray<FSlateVertex> Vertices;
    TArray<SlateIndex> Indices;

    FVector2D Size = AllottedGeometry.GetLocalSize();
    if (Size.X <= 0 || Size.Y <= 0) {
      return;
    }

    const int32 NumPoints = FMath::CeilToInt(Size.X);
    const float HalfHeight = Size.Y * 0.5f;
    const float Step = 1.0f / (NumPoints - 1);

    TArray<FVector2D> PointCache;
    GetPointCache(NumPoints, PointCache);

    FVector2D Pos = AllottedGeometry.GetLocalPositionAtCoordinates(FVector2D(0.0f));

    for (int32 i = 0; i < NumPoints; ++i) {
      float Frequency = i * Step;
      FVector2D Point(i + Pos.X, Pos.Y - 10.0f + HalfHeight - HalfHeight * EvaluateCurve(*MySpectrum, Frequency));
      PointCache[i] = Point;
    }

    FSlateDrawElement::MakeLines(
        OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), PointCache, ESlateDrawEffect::None, CurveColor);
  }
}

void SSpectrumWidget::DrawBorder(
    const FPaintArgs& Args,
    const FGeometry& AllottedGeometry,
    int32 LayerId,
    FSlateWindowElementList& OutDrawElements) const {
  // Draw the border lines around the padded geometry
  const FVector2f BorderPosition(0.0f);
  const FVector2f BorderSize(AllottedGeometry.GetLocalSize());

  // Define the four corners of the border
  const FVector2D TopLeft(BorderPosition.X, BorderPosition.Y);
  const FVector2D TopRight(BorderPosition.X + BorderSize.X, BorderPosition.Y);
  const FVector2D BottomRight(BorderPosition.X + BorderSize.X, BorderPosition.Y + BorderSize.Y);
  const FVector2D BottomLeft(BorderPosition.X, BorderPosition.Y + BorderSize.Y);
  const FVector2D BorderSize2D(BorderSize);

  const FSlateColorBrush WhiteBrush(GraphColor.Get());
  const FSlateBrush* TickBrush = &WhiteBrush;

  const FSlateLayoutTransform BorderTransform(1.0f, TopLeft);
  FSlateDrawElement::MakeBox(
      OutDrawElements,
      LayerId,
      AllottedGeometry.ToPaintGeometry(BorderSize2D, BorderTransform),
      TickBrush,
      ESlateDrawEffect::None,
      FLinearColor(.05f, .05f, .05f));

  if (!IsEnabled())
    return;

  // Draw the lines forming the border
  FSlateDrawElement::MakeLines(
      OutDrawElements,
      LayerId,
      AllottedGeometry.ToPaintGeometry(),
      TArray<FVector2D>{TopLeft, TopRight, BottomRight, BottomLeft, TopLeft},
      ESlateDrawEffect::None,
      FLinearColor::Black);
}

void SSpectrumWidget::DrawDataTicks(
    const FPaintArgs& Args,
    const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect,
    FSlateWindowElementList& OutDrawElements,
    int32 LayerId,
    const FWidgetStyle& InWidgetStyle) const {
  const int32 Ticks = 10;
  FVector2D Size = AllottedGeometry.GetLocalSize();

  const float TickHeight = Size.Y / Ticks;
  const float LabelOffsetX = 8.0f;
  const float TickOffsetX = 2.0f;
  const float TickWidth = 3.0f;

  const FSlateColorBrush WhiteBrush(GraphColor.Get());
  const FSlateBrush* SlateBrush = &WhiteBrush;

  for (int32 i = 0; i <= Ticks; i++) {
    float RelativePosition = 1.0f - static_cast<float>(i) / Ticks;
    float Value = MapData(RelativePosition, false);

    FVector2D TickPosition(Size.X + TickOffsetX, -Size.Y / (2 * Ticks) + i * TickHeight);
    TickPosition.Y += 4.0f;

    // Draw tick
    FSlateDrawElement::MakeBox(
        OutDrawElements,
        LayerId,
        AllottedGeometry.ToPaintGeometry(
            FVector2D(TickWidth, 1.5f), FSlateLayoutTransform(1.0f, FVector2D(TickPosition.X, TickPosition.Y))),
        SlateBrush,
        ESlateDrawEffect::None,
        FLinearColor::Gray);

    FVector2D LabelPosition(Size.X + LabelOffsetX, -Size.Y / (2 * Ticks) + i * TickHeight);

    // Draw label
    FString ValueString = FString::Printf(TEXT("%0.3f"), Value);
    FSlateFontInfo FontInfo = FAppStyle::GetFontStyle(TEXT("MenuItem.Font"));
    FontInfo.Size = 6;

    FSlateDrawElement::MakeText(
        OutDrawElements,
        LayerId,
        AllottedGeometry.ToPaintGeometry(FVector2D(4, 4), FSlateLayoutTransform(1.0f, FVector2D(LabelPosition.X, LabelPosition.Y))),
        FText::FromString(ValueString),
        FontInfo,
        ESlateDrawEffect::None,
        FLinearColor::Gray);
  }
}

void SSpectrumWidget::DrawFrequencyTicks(
    const FPaintArgs& Args,
    const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect,
    FSlateWindowElementList& OutDrawElements,
    int32 LayerId,
    const FWidgetStyle& InWidgetStyle) const {
  const int32 NumTicks = 28;
  FVector2D Size = AllottedGeometry.GetLocalSize();

  FSlateFontInfo FontInfo = FAppStyle::GetFontStyle(TEXT("MenuItem.Font"));
  FontInfo.Size = 7;

  float TickWidth = 1.0f;
  float TickHeight = Size.Y;
  const float LabelOffsetY = 16;
  const float LabelWidth = 4.0f;

  bool bEnabled = IsEnabled();

  FVector2D TickPosition;
  TickPosition.X = AllottedGeometry.Position.X;
  TickPosition.Y = AllottedGeometry.Position.Y;
  for (int32 i = 1; i < NumTicks; i++) {
    float Frequency;

    const FSlateColorBrush WhiteBrush(GraphColor.Get());
    const FSlateBrush* TickBrush = &WhiteBrush;

    if (MapFrequencyTick(i, Frequency)) {
      TickPosition.X = MapFrequency(Frequency) * Size.X;
      TickHeight = LabelOffsetY;
      TickWidth = 2;

      FSlateDrawElement::MakeBox(
          OutDrawElements,
          LayerId,
          AllottedGeometry.ToPaintGeometry(
              FVector2D(TickWidth, TickHeight), FSlateLayoutTransform(1.0f, FVector2D(TickPosition.X, TickPosition.Y))),
          TickBrush,
          ESlateDrawEffect::None,
          FLinearColor::Black);

      TickPosition.Y = 2 * LabelOffsetY;
      TickHeight = Size.Y - TickPosition.Y;

      FSlateDrawElement::MakeBox(
          OutDrawElements,
          LayerId,
          AllottedGeometry.ToPaintGeometry(
              FVector2D(TickWidth, TickHeight), FSlateLayoutTransform(1.0f, FVector2D(TickPosition.X, TickPosition.Y))),
          TickBrush,
          ESlateDrawEffect::None,
          FLinearColor::Black);

      // Draw label
      FVector2D LabelPosition(TickPosition.X - 2.0f, LabelOffsetY * 1.2);
      FString FrequencyString = FrequencyToTickString(Frequency);

      FSlateDrawElement::MakeText(
          OutDrawElements,
          LayerId + 1,
          AllottedGeometry.ToPaintGeometry(FVector2D(LabelWidth, LabelWidth), FSlateLayoutTransform(1.0f, LabelPosition)),
          FText::FromString(FrequencyString),
          FontInfo,
          ESlateDrawEffect::None,
          bEnabled ? FLinearColor::Black : FLinearColor(.015f, .015f, .015f));

      TickPosition.Y = AllottedGeometry.Position.Y;
      TickHeight = Size.Y;
      TickWidth = 1.0f;
    } else {
      TickPosition.X = MapFrequency(Frequency) * Size.X;
      TickPosition.Y = 0;

      // Draw tick above the label
      FSlateDrawElement::MakeBox(
          OutDrawElements,
          LayerId,
          AllottedGeometry.ToPaintGeometry(
              FVector2D(TickWidth, TickHeight), FSlateLayoutTransform(1.0f, FVector2D(TickPosition.X, TickPosition.Y))),
          TickBrush,
          ESlateDrawEffect::None,
          FLinearColor::Black);
    }
  }
}

int32 SSpectrumWidget::OnPaint(
    const FPaintArgs& Args,
    const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect,
    FSlateWindowElementList& OutDrawElements,
    int32 LayerId,
    const FWidgetStyle& InWidgetStyle,
    bool bParentEnabled) const {
  if (!MySpectrum) {
    return 0;
  }

  FVector2D Size = AllottedGeometry.GetLocalSize();
  Size.X -= 35.0f;
  Size.Y -= 20;
  FGeometry PaddedGeometry = AllottedGeometry.MakeChild(Size, FSlateLayoutTransform(1.0f, FVector2D(0, 10)));

  DrawBorder(Args, PaddedGeometry, LayerId++, OutDrawElements);
  DrawDataTicks(Args, PaddedGeometry, MyCullingRect, OutDrawElements, LayerId++, InWidgetStyle);
  DrawFrequencyTicks(Args, PaddedGeometry, MyCullingRect, OutDrawElements, LayerId++, InWidgetStyle);
  DrawMinMaxFilledCurve(Args, PaddedGeometry, MyCullingRect, OutDrawElements, LayerId++, InWidgetStyle, bParentEnabled, *MySpectrum);
  DrawCurve(Args, PaddedGeometry, LayerId++, OutDrawElements, GraphColor.Get());
  DrawSelected(*MySpectrum, PaddedGeometry, OutDrawElements, LayerId++);

  return 0;
}

FVector2D SSpectrumWidget::MapPointPosition(const FGeometry& AllottedGeometry, const FMetaXRAudioPoint& Point) const {
  float MappedX = MapFrequency(Point.Frequency);
  float MappedY = 1.0f - MapData(Point.Data);

  return AllottedGeometry.LocalToAbsolute(
      FVector2D(AllottedGeometry.GetLocalSize().X * MappedX, AllottedGeometry.GetLocalSize().Y * MappedY));
}

void SSpectrumWidget::DrawSelected(
    const FMetaXRAudioSpectrum& Spectrum,
    const FGeometry& AllottedGeometry,
    FSlateWindowElementList& OutDrawElements,
    int32 LayerId) const {
  if (Spectrum.Points.Num() > Spectrum.Selection) {
    const FMetaXRAudioPoint& Point = Spectrum.Points[Spectrum.Selection];

    FVector2D Position = AllottedGeometry.AbsoluteToLocal(MapPointPosition(AllottedGeometry, Point));
    FVector2D PointSize(8, 8);
    FVector2D PointRect;
    PointRect.X = Position.X - PointSize.X * 0.5f;
    PointRect.Y = Position.Y - PointSize.Y * 0.5f;

    const FSliderStyle* SliderStyle = &FCoreStyle::Get().GetWidgetStyle<FSliderStyle>("Slider");
    const FSlateBrush* TickBrush = &SliderStyle->NormalThumbImage;

    // Assuming you have a SlateBrush named MySlateBrush for the circle
    FSlateDrawElement::MakeBox(
        OutDrawElements,
        LayerId++,
        AllottedGeometry.ToPaintGeometry(FVector2D(8.0f), FSlateLayoutTransform(1.0f, PointRect)),
        TickBrush,
        ESlateDrawEffect::None,
        FLinearColor::White);

    const float LabelPadding = 2.0f;
    FVector2D LabelSize(8.0f, 32.0f);
    FVector2D LabelPosition(Position.X - LabelSize.X * 0.5f, Position.Y - PointSize.Y * 0.5f - LabelPadding - LabelSize.Y);

    FSlateDrawElement::MakeText(
        OutDrawElements,
        LayerId++,
        AllottedGeometry.ToPaintGeometry(LabelSize, FSlateLayoutTransform(1.0f, LabelPosition)),
        FText::FromString(FrequencyToString(Point.Frequency) + "\n" + DataToString(Point.Data)),
        FCoreStyle::GetDefaultFontStyle("Regular", 8),
        ESlateDrawEffect::None,
        FLinearColor::White);
  }
}

FReply SSpectrumWidget::OnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& MouseEvent) {
  if (!MySpectrum) {
    return FReply::Unhandled();
  }

  FVector2D Size = Geometry.GetLocalSize();
  Size.X -= 35.0f;
  Size.Y -= 20;
  FGeometry PaddedGeometry = Geometry.MakeChild(Size, FSlateLayoutTransform(1.0f, FVector2D(0, 10)));

  int32 Selection = MySpectrum->Selection;
  float MinDistance = MAX_FLT;
  FVector2D MousePosition = MouseEvent.GetScreenSpacePosition();
  for (int32 i = 0; i < MySpectrum->Points.Num(); i++) {
    float Distance = FVector2D::Distance(MapPointPosition(PaddedGeometry, MySpectrum->Points[i]), MousePosition);
    if (Distance < MinDistance) {
      Selection = i;
      MinDistance = Distance;
    }
  }

  MySpectrum->Selection = Selection;

  return FReply::Handled();
}

FReply SSpectrumWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
  // Reset the selected point index when the mouse button is released
  MySpectrum->Sort();
  MySpectrum->Selection = MAX_int32;
  return FReply::Handled();
}

FMetaXRAudioPoint SSpectrumWidget::MapMouseEvent(const FGeometry& Geometry, const FVector2D& MousePosition) {
  const FSlateRect BoundingRect = Geometry.GetLayoutBoundingRect();

  FMetaXRAudioPoint MappedPoint;

  MappedPoint.Frequency = (MousePosition.X < BoundingRect.Left) ? 0.0f
      : (MousePosition.X > BoundingRect.Right)                  ? FrequencyMax
                                               : MapFrequency((MousePosition.X - BoundingRect.Left) / BoundingRect.GetSize().X, false);

  MappedPoint.Data = (MousePosition.Y < BoundingRect.Top) ? RangeMax
      : (MousePosition.Y > BoundingRect.Bottom)           ? RangeMin
                                                : MapData(1 - (MousePosition.Y - BoundingRect.Top) / BoundingRect.GetSize().Y, false);

  return MappedPoint;
}

FReply SSpectrumWidget::OnMouseMove(const FGeometry& Geometry, const FPointerEvent& MouseEvent) {
  if (MySpectrum->Selection == MAX_int32)
    return FReply::Unhandled();

  FVector2D Size = Geometry.GetLocalSize();
  Size.X -= 35.0f;
  Size.Y -= 20;
  FGeometry PaddedGeometry = Geometry.MakeChild(Size, FSlateLayoutTransform(1.0f, FVector2D(0, 10)));

  FMetaXRAudioPoint NewPoint = MapMouseEvent(PaddedGeometry, MouseEvent.GetScreenSpacePosition());

  FMetaXRAudioSpectrum ModifiedSpectrum;
  ModifiedSpectrum.Clone(*MySpectrum);

  int32 SelectionIndex = ModifiedSpectrum.Selection;
  if (SelectionIndex >= 0 && SelectionIndex < ModifiedSpectrum.Points.Num()) {
    ModifiedSpectrum.Points[SelectionIndex].Frequency = NewPoint.Frequency;
  }

  SelectionIndex = ModifiedSpectrum.Selection;
  if (SelectionIndex >= 0 && SelectionIndex < ModifiedSpectrum.Points.Num()) {
    ModifiedSpectrum.Points[SelectionIndex].Data = NewPoint.Data;
  }

  MySpectrum->Clone(ModifiedSpectrum);
  MySpectrum->IsDirty = true;
  return FReply::Handled();
}

float SSpectrumWidget::MapData(float Value, bool bForward /*= true*/) const {
  float RangeSize = RangeMax - RangeMin;
  float Result = Value;

  if (bForward) {
    Result = (Value - RangeMin) / RangeSize;
  }

  switch (Scale) {
    case EAxisScale::Log:
      Result = bForward ? (Result < 1e-3f ? 0 : 1 + (FMath::LogX(10, Result) / 3)) : FMath::Pow(10, -3 * (1 - Result));
      break;

    case EAxisScale::Square:
      Result = bForward ? FMath::Sqrt(Result) : Result * Result;
      break;

    case EAxisScale::SquareCentered:
      if (bForward) {
        Result = Result > 0.5f ? 0.5f + 0.5f * FMath::Sqrt((Result - 0.5f) * 2.0f) : 0.5f - 0.5f * FMath::Sqrt((0.5f - Result) * 2.0f);
      } else {
        Result = Result > 0.5f ? 0.5f + 0.5f * FMath::Square((Result - 0.5f) * 2.0f) : 0.5f - 0.5f * FMath::Square((0.5f - Result) * 2.0f);
      }
      break;

    case EAxisScale::Cube:
      Result = bForward ? FMath::Pow(Result, 1.0f / 3.0f) : Result * Result * Result;
      break;

    default:
    case EAxisScale::Linear:
      break;
  }

  if (!bForward) {
    Result = Result * RangeSize + RangeMin;
  }

  return Result;
}

bool SSpectrumWidget::MapFrequencyTick(int32 i, float& Frequency) {
  int32 Power = i / 9 + 1;
  int32 Multiplier = i % 9 + 1;

  Frequency = Multiplier * static_cast<int32>(FMath::Pow(10.0f, Power));

  return Multiplier == 1;
}

FString SSpectrumWidget::FrequencyToString(float Frequency) {
  if (Frequency < 1000)
    return FString::Printf(TEXT("%.1f Hz"), Frequency);
  else
    return FString::Printf(TEXT("%.2f kHz"), Frequency * 0.001f);
}

FString SSpectrumWidget::DataToString(float Data) {
  return FString::Printf(TEXT("%.3f"), Data);
}

FString SSpectrumWidget::FrequencyToTickString(float Frequency) {
  if (Frequency < 1000) {
    return FString::Printf(TEXT("%0.0f Hz"), Frequency);
  } else {
    return FString::Printf(TEXT("%0.0f kHz"), Frequency * 0.001f);
  }
}
