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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FHedgeKernelTriangleTest, "Hedge.Kernel.CreateTriangle",
  EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter
)

bool FHedgeKernelOpAddRemoveTest::RunTest(FString const& Parameters)
{
  auto* Kernel = NewObject<UHedgeKernel>();
  auto const PIndex0 = Kernel->Add(FPoint(FVector(0.f, 0.f, 0.f)));
  auto const PIndex1 = Kernel->Add(FPoint(FVector(1.f, 0.f, 0.f)));
  auto const PIndex2 = Kernel->Add(FPoint(FVector(0.f, 1.f, 0.f)));
  auto const PIndex3 = Kernel->Add(FPoint(FVector(0.f, 0.f, 1.f)));

  // Make sure the most basic of operations isn't broken
  {
    auto const NumPoints = Kernel->NumPoints();
    TestEqual(TEXT("Added four points to mesh"), NumPoints, 4);
  }
  TestEqual(TEXT("Expected offset for the point0"), PIndex0.Offset(), 0);
  TestEqual(TEXT("Expected offset for the point1"), PIndex1.Offset(), 1);
  TestEqual(TEXT("Expected offset for the point2"), PIndex2.Offset(), 2);
  TestEqual(TEXT("Expected offset for the point3"), PIndex3.Offset(), 3);

  auto& Point1 = Kernel->Get(PIndex1);
  TestEqual(TEXT("No vertices associated yet"), Point1.Vertices.Num(), 0);
  TestEqual(TEXT("Expected value for the point1 X position."), Point1.Position.X, 1.f);
  TestEqual(TEXT("Expected value for the point1 Y position."), Point1.Position.Y, 0.f);
  TestEqual(TEXT("Expected value for the point1 Z position."), Point1.Position.Z, 0.f);

  Kernel->Remove(PIndex2);
  {
    auto const NumPoints = Kernel->NumPoints();
    TestEqual(TEXT("Expected 3 points after removing one."), NumPoints, 3);
  }
  TestFalse(TEXT("The kernel should report PIndex2 as invalid."), Kernel->IsValidIndex(PIndex2));
  auto const PIndex4 = Kernel->Add(FPoint(FVector(2.f, 2.f, 2.f)));
  TestEqual(TEXT("Expected offset for the next point added."), PIndex4.Offset(), 2);
  TestTrue(TEXT("The kernel should report PIndex2 as valid again."), Kernel->IsValidIndex(PIndex2));

  auto& Point4 = Kernel->Get(PIndex2);
  TestEqual(TEXT("Expected value for the point4 X position."), Point4.Position.X, 2.f);
  TestEqual(TEXT("Expected value for the point4 Y position."), Point4.Position.Y, 2.f);
  TestEqual(TEXT("Expected value for the point4 Z position."), Point4.Position.Z, 2.f);

  return true;
}

bool FHedgeKernelTriangleTest::RunTest(const FString& Parameters)
{
  auto* Kernel = NewObject<UHedgeKernel>();

  auto const PIndex0 = Kernel->Add(FPoint(FVector(0.f, 0.f, 0.f)));
  auto const PIndex1 = Kernel->Add(FPoint(FVector(1.f, 0.f, 0.f)));
  auto const PIndex2 = Kernel->Add(FPoint(FVector(0.f, 1.f, 0.f)));
  auto& P0 = Kernel->Get(PIndex0);
  auto& P1 = Kernel->Get(PIndex1);
  auto& P2 = Kernel->Get(PIndex2);

  auto const EIndex0 = Kernel->MakeEdgePair();
  auto const EIndex1 = Kernel->MakeEdgePair();
  auto const EIndex2 = Kernel->MakeEdgePair();

  auto const VIndex0 = Kernel->ConnectEdges(EIndex0, PIndex1, EIndex1);
  auto const VIndex1 = Kernel->ConnectEdges(EIndex1, PIndex2, EIndex2);
  auto const VIndex2 = Kernel->ConnectEdges(EIndex2, PIndex0, EIndex0);

  auto const FIndex0 = Kernel->MakeFace(EIndex0);

  TestEqual(TEXT("Unexpected number of points"), Kernel->NumPoints(), 3);
  TestEqual(TEXT("Unexpected number of edges"), Kernel->NumEdges(), 6);
  TestEqual(TEXT("Unexpected number of vertices"), Kernel->NumVertices(), 3);
  TestEqual(TEXT("Unexpected number of faces"), Kernel->NumFaces(), 1);

  TestTrue(TEXT("P0 was missing vertex2 index"), P0.Vertices.Contains(VIndex2));
  TestTrue(TEXT("P1 was missing vertex0 index"), P1.Vertices.Contains(VIndex0));
  TestTrue(TEXT("P2 was missing vertex1 index"), P2.Vertices.Contains(VIndex1));

  auto CheckEdge = [&Kernel, this](
    FEdgeIndex const Eindex, 
    FEdgeIndex const Prev,
    FEdgeIndex const Next, 
    FVertexIndex const Vindex,
    FFaceIndex const Findex)
  {
    auto& Edge = Kernel->Get(Eindex);
    TestTrue(TEXT("Edge points to invalid next edge."), Kernel->IsValidIndex(Next));
    TestTrue(TEXT("Edge points to invalid prev edge."), Kernel->IsValidIndex(Prev));
    TestEqual(TEXT("Edge face index did not match specified face."), Edge.FaceIndex, Findex);
    TestEqual(TEXT("Edge prev index did not match specified edge."), Edge.PrevEdgeIndex, Prev);
    TestEqual(TEXT("Edge next index did not match specified edge."), Edge.NextEdgeIndex, Next);
    TestEqual(TEXT("Edge vertex index did not match specified edge."), Edge.VertexIndex, Vindex);
  };

  CheckEdge(EIndex0, EIndex2, EIndex1, VIndex2, FIndex0);
  CheckEdge(EIndex1, EIndex0, EIndex2, VIndex0, FIndex0);
  CheckEdge(EIndex2, EIndex1, EIndex0, VIndex1, FIndex0);

  return true;
}


#endif
