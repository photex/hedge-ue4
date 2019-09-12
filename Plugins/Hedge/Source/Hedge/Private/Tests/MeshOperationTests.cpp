// Copyright 2019 Chip Collier. All Rights Reserved.

#include "CoreTypes.h"
#include "Misc/AutomationTest.h"
#include "HedgeTypes.h"
#include "HedgeElements.h"
#include "HedgeMesh.h"
#include "HedgeProxies.h"
#include "HedgeLogging.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FHedgeMeshFaceTest, "Hedge.Mesh.AddFaces",
  EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FHedgeMeshFaceTest::RunTest(const FString& Parameters)
{
  auto* Mesh = NewObject<UHedgeMesh>();
  TArray<FVector> const Positions = {
    FVector(-1.0f, 0.0f, 1.0f),
    FVector(-1.0f, 0.0f, -1.0f),
    FVector(1.0f, -1.0f, 0.0f),
    FVector(1.0f, 1.0f, 0.0f),
  };
  auto Points = Mesh->AddPoints(Positions);

  ///////////////////////////////////////////////////////////////////
  // Build a tetrahedron using the points added to the mesh

  // Triangle from p[0,1,3]
  auto const F0 = Mesh->AddFace({
    Points[0],
    Points[1],
    Points[3]
  });

  // Triangle from p[1,0,2]
  auto const F1 = Mesh->AddFace(
    Mesh->Face(F0).RootEdge().Adjacent().GetHandle(),
    Points[2]
  );

  // Triangle from p[2,0,3]
  auto const F2 = Mesh->AddFace({
    Mesh->Face(F1).RootEdge().Next().Adjacent().GetHandle(),
    Mesh->Face(F0).RootEdge().Prev().Adjacent().GetHandle(),
  });

  // Triangle from p[2,3,1]
  auto const F3 = Mesh->AddFace({
    Mesh->Face(F2).RootEdge().Prev().Adjacent().GetHandle(),
    Mesh->Face(F0).RootEdge().Next().Adjacent().GetHandle(),
    Mesh->Face(F1).RootEdge().Prev().Adjacent().GetHandle(),
  });

  FHedgeMeshStats Stats;
  Mesh->GetStats(Stats);

  //////////////////////////////////////////////////////////////////
  // Make sure the tetrahedron was constructed as expected

  auto const TestPerimeterPoints = [this](
    TArray<FPxHalfEdge> const& PerimeterEdges,
    uint32 const P0, uint32 const P1, uint32 const P2)
  {
    auto const E0Points = PerimeterEdges[0].GetPoints();
    auto const E1Points = PerimeterEdges[1].GetPoints();
    auto const E2Points = PerimeterEdges[2].GetPoints();

    auto const E0P0 = E0Points[0].GetHandle();
    auto const E0P1 = E0Points[1].GetHandle();
    auto const E1P0 = E1Points[0].GetHandle();
    auto const E1P1 = E1Points[1].GetHandle();
    auto const E2P0 = E2Points[0].GetHandle();
    auto const E2P1 = E2Points[1].GetHandle();

    TestEqual(TEXT("E0P0 == P0"), E0P0.GetIndex(), P0);
    TestEqual(TEXT("E1P0 == P1"), E1P0.GetIndex(), P1);
    TestEqual(TEXT("E2P0 == P3"), E2P0.GetIndex(), P2);

    TestEqual(TEXT("E0P1 == E1P0"), E0P1, E1P0);
    TestEqual(TEXT("E1P1 == E2P0"), E1P1, E2P0);
    TestEqual(TEXT("E2P1 == E0P0"), E2P1, E0P0);
  };

  TestEqual(TEXT("Mesh consists of 4 faces."), Stats.NumFaces, 4);
  for (FPxFace CurrentFace : Mesh->Faces())
  {
    DebugLogV("Examining Face %s", *CurrentFace.GetHandle().ToString());

    auto const PerimeterEdges = CurrentFace.GetPerimeterEdges();
    TestEqual(TEXT("Perimeter edge count == 3"), PerimeterEdges.Num(), 3);

    auto const CurrentIndex = CurrentFace.GetHandle().GetIndex();
    switch (CurrentIndex)
    {
      case 0:
      {
        TestPerimeterPoints(PerimeterEdges, 0, 1, 3);
        break;
      }

      case 1:
      {
        TestPerimeterPoints(PerimeterEdges, 1, 0, 2);
        break;
      }

      case 2:
      {
        TestPerimeterPoints(PerimeterEdges, 2, 0, 3);
        break;
      }

      case 3:
      {
        TestPerimeterPoints(PerimeterEdges, 2, 3, 1);
        break;
      }

      default:
      {
        AddError(FString::Printf(TEXT("Unexpected face index %d"), CurrentIndex), 0);
        break;
      }
    }
  }

  TestEqual(TEXT("Mesh consists of 12 edges."), Stats.NumEdges, 12);
  for (FPxHalfEdge CurrentEdge : Mesh->Edges())
  {
    auto const CurrentHandle = CurrentEdge.GetHandle();
    DebugLogV("Examining Edge %s", *CurrentHandle.ToString());
    TestFalse(FString::Printf(TEXT("Edge %d is a boundary edge."), 
      CurrentHandle.GetIndex()), CurrentEdge.IsBoundary());
  }

  TestEqual(TEXT("Mesh consists of 12 vertices."), Stats.NumVertices, 12);
  for (FPxVertex CurrentVertex : Mesh->Vertices())
  {
    DebugLogV("Examining Vertex %s", *CurrentVertex.GetHandle().ToString());
  }

  TestEqual(TEXT("Mesh consists of 4 points."), Stats.NumPoints, 4);
  for (FPxPoint CurrentPoint : Mesh->Points())
  {
    DebugLogV("Examining Point %s", *CurrentPoint.GetHandle().ToString());

    auto& Vertices = CurrentPoint.Vertices();
    TestEqual(TEXT("Point has 3 associated vertices"), Vertices.Num(), 3);

    // TODO: Fix hashing of handles so that we don't require the generation here.
    auto const CurrentIndex = CurrentPoint.GetHandle().GetIndex();
    switch (CurrentIndex)
    {
      case 0:
      {
        TestTrue(TEXT("P0 -> V0"), Vertices.Contains(Mesh->Edge(0).Vertex().GetHandle()));
        TestTrue(TEXT("P0 -> V5"), Vertices.Contains(Mesh->Edge(5).Vertex().GetHandle()));
        TestTrue(TEXT("P0 -> V6"), Vertices.Contains(Mesh->Edge(6).Vertex().GetHandle()));
        break;
      }

      case 1:
      {
        TestTrue(TEXT("P1 -> V2"), Vertices.Contains(FVertexHandle(2, 1)));
        TestTrue(TEXT("P1 -> V1"), Vertices.Contains(FVertexHandle(1, 1)));
        TestTrue(TEXT("P1 -> V9"), Vertices.Contains(FVertexHandle(9, 1)));
        break;
      }

      case 2:
      {
        TestTrue(TEXT("P2 -> V7"), Vertices.Contains(FVertexHandle(7)));
        TestTrue(TEXT("P2 -> V8"), Vertices.Contains(FVertexHandle(8)));
        TestTrue(TEXT("P2 -> V11"), Vertices.Contains(FVertexHandle(11)));
        break;
      }

      case 3:
      {
        TestTrue(TEXT("P3 -> V4"), Vertices.Contains(4));
        TestTrue(TEXT("P3 -> V3"), Vertices.Contains(3));
        TestTrue(TEXT("P3 -> V10"), Vertices.Contains(10));
        break;
      }

      default:
      {
        AddError(FString::Printf(TEXT("Unexpected point index %d"), CurrentIndex), 0);
        break;
      }
    }
  }

  return true;
}


#endif // WITH_DEV_AUTOMATION_TESTS
