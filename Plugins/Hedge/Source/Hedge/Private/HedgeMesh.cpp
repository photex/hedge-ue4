// Copyright 2019 Chip Collier. All Rights Reserved.

#include "HedgeMesh.h"
#include "HedgeKernel.h"
#include "HedgeElements.h"
#include "HedgeProxies.h"
#include "HedgeLogging.h"

UHedgeMesh::UHedgeMesh()
  : Kernel(nullptr)
{
  Kernel = CreateDefaultSubobject<UHedgeKernel>(TEXT("MeshKernel"));
}

void UHedgeMesh::GetStats(FHedgeMeshStats& OutStats) const
{
  OutStats.NumPoints = Kernel->NumPoints();
  OutStats.NumVertices = Kernel->NumVertices();
  OutStats.NumEdges = Kernel->NumEdges();
  OutStats.NumFaces = Kernel->NumFaces();
}

FPxFace UHedgeMesh::Face(FFaceIndex const& Index) const
{
  return FPxFace(Kernel, Index);
}

FPxHalfEdge UHedgeMesh::Edge(FEdgeIndex const& Index) const
{
  return FPxHalfEdge(Kernel, Index);
}

FPxPoint UHedgeMesh::Point(FPointIndex const& Index) const
{
  return FPxPoint(Kernel, Index);
}

FPxVertex UHedgeMesh::Vertex(FVertexIndex const& Index) const
{
  return FPxVertex(Kernel, Index);
}

TArray<FPointIndex> UHedgeMesh::AddPoints(
  TArray<FVector> const& Positions) const
{
  auto const PositionCount = Positions.Num();
  return AddPoints(Positions.GetData(), PositionCount);
}

TArray<FPointIndex> UHedgeMesh::AddPoints(
  FVector const Positions[], 
  uint32 const PositionCount) const
{
  TArray<FPointIndex> OutPointIndices;
  OutPointIndices.Reserve(PositionCount);
  for (uint32 i = 0; i < PositionCount; ++i)
  {
    auto PointIndex = Kernel->Add(FPoint(Positions[i]));
    OutPointIndices.Push(PointIndex);
  }
  return MoveTemp(OutPointIndices);
}

FFaceIndex UHedgeMesh::AddFace(TArray<FPointIndex> const& Points)
{
  if (Points.Num() < 3)
  {
    ErrorLog("Unable to add a new face to mesh without at least 3 points.");
    return FFaceIndex::Invalid;
  }
  FFaceIndex FaceIndex;
  FFace& Face = Kernel->New(FaceIndex);
  
  FPointIndex const RootPoint = Points[0];
  FEdgeIndex const RootEdge = Kernel->MakeEdgePair();
  auto PreviousEdge = RootEdge;
  for (auto i = 1; i < Points.Num(); ++i)
  {
    FPointIndex const CurrentPoint = Points[i];
    FEdgeIndex const CurrentEdge = Kernel->MakeEdgePair(FaceIndex);

    Kernel->ConnectEdges(PreviousEdge, CurrentPoint, CurrentEdge);

    PreviousEdge = CurrentEdge;
  }
  Kernel->ConnectEdges(PreviousEdge, RootPoint, RootEdge);
  Face.RootEdgeIndex = RootEdge;
  return FaceIndex;
}

FFaceIndex UHedgeMesh::AddFace(
  FEdgeIndex const& RootEdge, TArray<FPointIndex> const& Points)
{
  if (Points.Num() < 1)
  {
    ErrorLog("Unable to add a new face to mesh without at least 1 point.");
    return FFaceIndex::Invalid;
  }
  FFaceIndex FaceIndex;
  FFace& Face = Kernel->New(FaceIndex);

  auto const RootPoint = Edge(RootEdge).Adjacent().Vertex().Point().GetIndex();
  auto CurrentEdge = RootEdge;
  auto CurrentPoint = RootPoint;
  auto PreviousEdge = RootEdge;
  auto PreviousPoint = RootPoint;
  for (auto i = 0; i < Points.Num(); ++i)
  {
    CurrentPoint = Points[i];
    CurrentEdge = Kernel->MakeEdgePair(FaceIndex);
    Kernel->ConnectEdges(PreviousEdge, PreviousPoint, CurrentEdge);

    PreviousEdge = CurrentEdge;
    PreviousPoint = CurrentPoint;
  }
  CurrentPoint = Edge(RootEdge).Adjacent().Next().Vertex().Point().GetIndex();
  CurrentEdge = Kernel->MakeEdgePair(FaceIndex);
  Kernel->ConnectEdges(PreviousEdge, PreviousPoint, CurrentEdge);
  Kernel->ConnectEdges(CurrentEdge, CurrentPoint, RootEdge);
  Face.RootEdgeIndex = RootEdge;
  return FaceIndex;
}

FFaceIndex UHedgeMesh::AddFace(
  FEdgeIndex const& E0, FPointIndex const& P1)
{
  FFaceIndex FaceIndex;
  FFace& Face = Kernel->New(FaceIndex);
  Face.RootEdgeIndex = E0;

  auto& Edge0 = Kernel->Get(E0);
  Edge0.FaceIndex = FaceIndex;

  auto const P0 = Edge(E0).Adjacent().Vertex().Point().GetIndex();
  auto const P2 = Edge(E0).Adjacent().Next().Vertex().Point().GetIndex();

  auto const E1 = Kernel->MakeEdgePair(FaceIndex);
  auto const E2 = Kernel->MakeEdgePair(FaceIndex);

  Kernel->ConnectEdges(E0, P0, E1);
  Kernel->ConnectEdges(E1, P1, E2);
  Kernel->ConnectEdges(E2, P2, E0);

  return FaceIndex;
}

FFaceIndex UHedgeMesh::AddFace(TArray<FEdgeIndex> const& Edges)
{
  if (Edges.Num() < 3)
  {
    ErrorLog("Unable to create a face without at least 3 edges.");
    return FFaceIndex::Invalid;
  }
  auto const RootEdge = Edges[0];
  auto LastEdge = RootEdge;
  auto CurrentEdge = LastEdge;
  for (auto i = 1; i < Edges.Num(); ++i)
  {
    CurrentEdge = Edges[i];
    auto const Point = Edge(LastEdge).Adjacent().Vertex().Point().GetIndex();
    Kernel->ConnectEdges(LastEdge, Point, CurrentEdge);
    LastEdge = CurrentEdge;
  }
  auto const Point = Edge(LastEdge).Adjacent().Vertex().Point().GetIndex();
  Kernel->ConnectEdges(LastEdge, Point, RootEdge);
  // Maybe work this into our already existing loop?
  return Kernel->MakeFace(RootEdge);
}

void UHedgeMesh::Dissolve(FEdgeIndex Index)
{
  unimplemented();
}

void UHedgeMesh::Dissolve(FFaceIndex Index)
{
  unimplemented();
}

void UHedgeMesh::Dissolve(FVertexIndex Index)
{
  unimplemented();
}

void UHedgeMesh::Dissolve(FPointIndex Index)
{
  unimplemented();
}
