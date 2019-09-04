// Copyright 2019 Chip Collier. All Rights Reserved.

#include "CoreTypes.h"
#include "Misc/AutomationTest.h"
#include "HedgeTypes.h"
#include "HedgeElements.h"
#include "HedgeKernel.h"

#if WITH_DEV_AUTOMATION_TESTS

///////////////////////////////////////////////////////////
/// Add and remove elements and verify the expected
/// behaviors and side-effects hold true.

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FHedgeKernelOpAddRemoveTest, "Hedge.Kernel.AddAndRemoveElements",
  EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
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
  TestEqual(TEXT("Expected offset for the point0"), PIndex0.GetIndex(), 0);
  TestEqual(TEXT("Expected offset for the point1"), PIndex1.GetIndex(), 1);
  TestEqual(TEXT("Expected offset for the point2"), PIndex2.GetIndex(), 2);
  TestEqual(TEXT("Expected offset for the point3"), PIndex3.GetIndex(), 3);

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
  TestFalse(
    TEXT("The kernel should report PIndex2 as invalid."), Kernel->IsValidHandle(PIndex2));
  auto const PIndex4 = Kernel->Add(FPoint(FVector(2.f, 2.f, 2.f)));
  TestEqual(TEXT("Expected offset for the next point added."), PIndex4.GetIndex(), 2);
  TestTrue(
    TEXT("The kernel should report PIndex2 as valid again."),
    Kernel->IsValidHandle(PIndex2));

  auto& Point4 = Kernel->Get(PIndex2);
  TestEqual(TEXT("Expected value for the point4 X position."), Point4.Position.X, 2.f);
  TestEqual(TEXT("Expected value for the point4 Y position."), Point4.Position.Y, 2.f);
  TestEqual(TEXT("Expected value for the point4 Z position."), Point4.Position.Z, 2.f);

  return true;
}

///////////////////////////////////////////////////////////
/// Build a triangle using only facilities from the Kernel
/// and verify the connectivity information is correct.

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FHedgeKernelTriangleTest, "Hedge.Kernel.CreateTriangle",
  EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FHedgeKernelTriangleTest::RunTest(const FString& Parameters)
{
  auto* Kernel = NewObject<UHedgeKernel>();

  FPointHandle PIndex0, PIndex1, PIndex2;
  auto& P0 = Kernel->New(PIndex0, FVector(0.f, 0.f, 0.f));
  auto& P1 = Kernel->New(PIndex1, FVector(1.f, 0.f, 0.f));
  auto& P2 = Kernel->New(PIndex2, FVector(0.f, 1.f, 0.f));

  FFaceHandle FIndex0;
  auto& Face = Kernel->New(FIndex0);

  auto const EIndex0 = Kernel->MakeEdgePair(,, FIndex0);
  auto const EIndex1 = Kernel->MakeEdgePair(,, FIndex0);
  auto const EIndex2 = Kernel->MakeEdgePair(,, FIndex0);

  auto const VIndex0 = Kernel->ConnectEdges(EIndex0, PIndex1, EIndex1);
  auto const VIndex1 = Kernel->ConnectEdges(EIndex1, PIndex2, EIndex2);
  auto const VIndex2 = Kernel->ConnectEdges(EIndex2, PIndex0, EIndex0);

  Face.RootEdge = EIndex0;

  TestEqual(TEXT("Unexpected number of points"), Kernel->NumPoints(), 3);
  TestEqual(TEXT("Unexpected number of edges"), Kernel->NumEdges(), 6);
  TestEqual(TEXT("Unexpected number of vertices"), Kernel->NumVertices(), 3);
  TestEqual(TEXT("Unexpected number of faces"), Kernel->NumFaces(), 1);

  TestTrue(TEXT("P0 was missing vertex2 index"), P0.Vertices.Contains(VIndex2));
  TestTrue(TEXT("P1 was missing vertex0 index"), P1.Vertices.Contains(VIndex0));
  TestTrue(TEXT("P2 was missing vertex1 index"), P2.Vertices.Contains(VIndex1));

  auto const ValidateEdge = [&Kernel, this](
    FEdgeHandle const Eindex,
    FEdgeHandle const Prev,
    FEdgeHandle const Next,
    FVertexHandle const Vindex,
    FFaceHandle const Findex)
  {
    auto& Edge = Kernel->Get(Eindex);
    TestTrue(TEXT("Edge points to invalid next edge."), Kernel->IsValidHandle(Next));
    TestTrue(TEXT("Edge points to invalid prev edge."), Kernel->IsValidHandle(Prev));
    TestEqual(
      TEXT("Edge face index did not match specified face."), Edge.Face, Findex);
    TestEqual(
      TEXT("Edge prev index did not match specified edge."), Edge.PrevEdge, Prev);
    TestEqual(
      TEXT("Edge next index did not match specified edge."), Edge.NextEdge, Next);
    TestEqual(
      TEXT("Edge vertex index did not match specified edge."), Edge.Vertex, Vindex);
  };

  ValidateEdge(EIndex0, EIndex2, EIndex1, VIndex2, FIndex0);
  ValidateEdge(EIndex1, EIndex0, EIndex2, VIndex0, FIndex0);
  ValidateEdge(EIndex2, EIndex1, EIndex0, VIndex1, FIndex0);

  return true;
}

///////////////////////////////////////////////////////////
/// Build a triangle after adding and removing elements
/// so that we've artificially created 'gaps' in the
/// element buffer. Then defrag the buffers and verify
/// that the elements are correctly remapped.

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FHedgeKernelDefragTest, "Hedge.Kernel.Defrag",
  EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FHedgeKernelDefragTest::RunTest(const FString& Parameters)
{
  auto* Kernel = NewObject<UHedgeKernel>();

  FPointHandle PIndex0, PIndex1, PIndex2;
  {
    FPointHandle PIndex[5];
    FPoint Points[5] = {
      Kernel->New(PIndex[0], FVector(-0.5f, 0.f, 0.f)),
      Kernel->New(PIndex[1], FVector(0.f, 1.f, 0.f)),
      Kernel->New(PIndex[2], FVector(1.f, 1.f, 0.f)),
      Kernel->New(PIndex[3], FVector(0.f, 1.f, 0.f)),
      Kernel->New(PIndex[4], FVector(0.5f, 0.f, 0.f)),
    };
    PIndex0 = PIndex[0];
    PIndex1 = PIndex[2];
    PIndex2 = PIndex[4];
    Kernel->Remove(PIndex[1]);
    Kernel->Remove(PIndex[3]);
  }
  TestEqual(TEXT("PIndex0 != 0."), PIndex0.GetIndex(), 0);
  TestEqual(TEXT("PIndex1 != 2."), PIndex1.GetIndex(), 2);
  TestEqual(TEXT("PIndex2 != 4."), PIndex2.GetIndex(), 4);

  FFaceHandle FIndex0;
  {
    FFaceHandle FIndex[3];
    FFace Faces[3] = {
      Kernel->New(FIndex[0]),
      Kernel->New(FIndex[1]),
      Kernel->New(FIndex[2]),
    };
    FIndex0 = FIndex[2];
    Kernel->Remove(FIndex[0]);
    Kernel->Remove(FIndex[1]);
  }
  TestEqual(TEXT("FIndex0 != 2"), FIndex0.GetIndex(), 2);

  FEdgeHandle EIndex0, EIndex1, EIndex2;
  {
    FEdgeHandle EIndex[5] = {
      Kernel->MakeEdgePair(,, FIndex0),
      Kernel->MakeEdgePair(,, FIndex0),
      Kernel->MakeEdgePair(,, FIndex0),
      Kernel->MakeEdgePair(,, FIndex0),
      Kernel->MakeEdgePair(,, FIndex0),
    };
    EIndex0 = EIndex[0];
    EIndex1 = EIndex[3];
    EIndex2 = EIndex[4];
    Kernel->Remove(EIndex[1]);
    Kernel->Remove(EIndex[2]);
  }
  TestEqual(TEXT("EIndex0 != 0"), EIndex0.GetIndex(), 0);
  TestEqual(TEXT("EIndex1 != 6"), EIndex1.GetIndex(), 6);
  TestEqual(TEXT("EIndex2 != 8"), EIndex2.GetIndex(), 8);

  Kernel->Get(FIndex0).RootEdge = EIndex0;

  Kernel->ConnectEdges(EIndex0, PIndex1, EIndex1);
  Kernel->ConnectEdges(EIndex1, PIndex2, EIndex2);
  Kernel->ConnectEdges(EIndex2, PIndex0, EIndex0);

  TestEqual(TEXT("NumPoints == 3"), Kernel->NumPoints(), 3);
  TestEqual(TEXT("NumEdges == 3"), Kernel->NumEdges(), 6);
  TestEqual(TEXT("NumFaces == 1"), Kernel->NumFaces(), 1);
  TestEqual(TEXT("NumVertices == 3"), Kernel->NumVertices(), 3);

  Kernel->Defrag();

  // Once we 'defrag' the element buffers, all existing indices should be
  // invalid because they'll have been created with the previous generation
  // value.

  TestFalse(TEXT("PIndex0 is still valid."), Kernel->IsValidHandle(PIndex0));
  TestFalse(TEXT("PIndex1 is still valid."), Kernel->IsValidHandle(PIndex1)); 
  TestFalse(TEXT("PIndex2 is still valid."), Kernel->IsValidHandle(PIndex2));

  TestFalse(TEXT("FIndex0 is still valid."), Kernel->IsValidHandle(FIndex0));

  TestFalse(TEXT("EIndex0 is still valid."), Kernel->IsValidHandle(EIndex0));
  TestFalse(TEXT("EIndex1 is still valid."), Kernel->IsValidHandle(EIndex1));
  TestFalse(TEXT("EIndex2 is still valid."), Kernel->IsValidHandle(EIndex2));

  // For the indexes that we expect to still point to data in the  kernel
  // we can just create a new version without the generation value.
  PIndex0 = FPointHandle(PIndex0.GetIndex());
  PIndex1 = FPointHandle(1);
  PIndex2 = FPointHandle(2);
  TestTrue(TEXT("Updated PIndex0 is valid."), Kernel->IsValidHandle(PIndex0));
  TestTrue(TEXT("Updated PIndex1 is valid."), Kernel->IsValidHandle(PIndex1));
  TestTrue(TEXT("Updated PIndex2 is valid."), Kernel->IsValidHandle(PIndex2));

  // Check that we get the expected position from PIndex2
  FVector PIndex2Position = Kernel->Get(PIndex2).Position;
  TestEqual(TEXT("PIndex2.Position.X == 0.5f"), PIndex2Position.X, 0.5f);
  TestEqual(TEXT("PIndex2.Position.Y == 0.0f"), PIndex2Position.Y, 0.0f);
  TestEqual(TEXT("PIndex2.Position.Z == 0.0f"), PIndex2Position.Z, 0.0f);

  // Check that our edges have indexes with the current generation
  uint32 const ExpectedGeneration = 2;
  EIndex0 = FEdgeHandle(0, ExpectedGeneration);
  EIndex1 = FEdgeHandle(1, ExpectedGeneration);
  auto& Edge0 = Kernel->Get(EIndex0);
  auto& Edge1 = Kernel->Get(EIndex1);
  TestEqual(TEXT("Edge0.NextEdgeIndex.Generation == 2"), 
    Edge0.NextEdge.GetGeneration(), ExpectedGeneration);
  TestEqual(TEXT("Edge1.PrevEdgeIndex.Generation == 2"), 
    Edge1.PrevEdge.GetGeneration(), ExpectedGeneration);
  TestTrue(TEXT("Edge0.NextEdgeIndex == EIndex1"), Edge0.NextEdge == EIndex1);
  TestTrue(TEXT("Edge1.PrevEdgeIndex == EIndex0"), Edge1.PrevEdge == EIndex0);

  // Our mesh stats shouldn't have changed
  TestEqual(TEXT("NumPoints == 3"), Kernel->NumPoints(), 3);
  TestEqual(TEXT("NumEdges == 3"), Kernel->NumEdges(), 6);
  TestEqual(TEXT("NumFaces == 1"), Kernel->NumFaces(), 1);
  TestEqual(TEXT("NumVertices == 3"), Kernel->NumVertices(), 3);

  return true;
}


#endif
