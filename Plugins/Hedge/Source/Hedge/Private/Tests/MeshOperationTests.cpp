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


  //////////////////////////////////////////////////////////////////
  // Make sure the tetrahedron was constructed as expected

  for (FPxFace CurrentFace : Mesh->Faces())
  {
    InfoLogV("Examining Face %s", *CurrentFace.GetHandle().ToString());

    auto const PerimeterEdges = CurrentFace.GetPerimeterEdges();
    TestEqual(TEXT("Permiter edge count == 3"), PerimeterEdges.Num(), 3);

    auto const E0Points = PerimeterEdges[0].GetPoints();
    auto const E1Points = PerimeterEdges[1].GetPoints();
    auto const E2Points = PerimeterEdges[2].GetPoints();

    auto const CurrentIndex = CurrentFace.GetHandle().GetIndex();
    switch (CurrentIndex)
    {
      case 0:
      {
        auto const E0P0 = E0Points[0].GetHandle();
        auto const E0P1 = E0Points[1].GetHandle();
        auto const E1P0 = E1Points[0].GetHandle();
        auto const E1P1 = E1Points[1].GetHandle();
        auto const E2P0 = E2Points[0].GetHandle();
        auto const E2P1 = E2Points[1].GetHandle();

        TestEqual(TEXT("E0P0 == P0"), E0P0.GetIndex(), 0);
        TestEqual(TEXT("E1P0 == P1"), E1P0.GetIndex(), 1);
        TestEqual(TEXT("E2P0 == P3"), E2P0.GetIndex(), 3);

        TestEqual(TEXT("E0P1 == E1P0"), E0P1, E1P0);
        TestEqual(TEXT("E1P1 == E2P0"), E1P1, E2P0);
        TestEqual(TEXT("E2P1 == E0P0"), E2P1, E0P0);

        break;
      }
      case 1:
        break;
      case 2:
        break;
      case 3:
        break;
      default:
        break;
    }
  }

  for (FPxHalfEdge CurrentEdge : Mesh->Edges())
  {
    InfoLogV("Examining Edge %s", *CurrentEdge.GetHandle().ToString());
  }

  for (FPxVertex CurrentVertex : Mesh->Vertices())
  {
    InfoLogV("Examining Vertex %s", *CurrentVertex.GetHandle().ToString());
  }

  for (FPxPoint CurrentPoint : Mesh->Points())
  {
    InfoLogV("Examining Point %s", *CurrentPoint.GetHandle().ToString());
  }

  return true;
}


#endif // WITH_DEV_AUTOMATION_TESTS
