// Copyright 2019 Chip Collier. All Rights Reserved.

#include "CoreTypes.h"
#include "Misc/AutomationTest.h"
#include "HedgeTypes.h"
#include "HedgeElements.h"
#include "HedgeKernel.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FHedgeKernelOpAddRemoveTest, "Hedge.Kernel.AddAndRemoveElements",
  EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter
)

bool FHedgeKernelOpAddRemoveTest::RunTest(FString const& Parameters)
{
  auto* Kernel = NewObject<UHedgeKernel>();
  auto PIndex0 = Kernel->Add(FPoint(FVector(0.f, 0.f, 0.f)));
  auto PIndex1 = Kernel->Add(FPoint(FVector(1.f, 0.f, 0.f)));
  auto PIndex2 = Kernel->Add(FPoint(FVector(0.f, 1.f, 0.f)));
  auto PIndex3 = Kernel->Add(FPoint(FVector(0.f, 0.f, 1.f)));

  // Make sure the most basic of operations isn't broken
  auto const NumPoints = Kernel->NumPoints();
  TestEqual(TEXT("Added four points to mesh"), NumPoints, 4);
  TestEqual(TEXT("Expected offset for the point0"), PIndex0.Offset(), 0);
  TestEqual(TEXT("Expected offset for the point1"), PIndex1.Offset(), 1);
  TestEqual(TEXT("Expected offset for the point2"), PIndex2.Offset(), 2);
  TestEqual(TEXT("Expected offset for the point3"), PIndex3.Offset(), 3);

  auto& Point1 = Kernel->Get(PIndex1);
  TestEqual(TEXT("No vertices associated yet"), Point1.Vertices.Num(), 0);
  TestEqual(TEXT("Expected value for the point1 X position."), Point1.Position.X, 1.f);
  TestEqual(TEXT("Expected value for the point1 Y position."), Point1.Position.Y, 0.f);
  TestEqual(TEXT("Expected value for the point1 Z position."), Point1.Position.Z, 0.f);

  return true;
}


#endif