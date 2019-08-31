// Copyright 2019 Chip Collier. All Rights Reserved.

#include "HedgeMesh.h"
#include "HedgeKernel.h"
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
  for (auto i = 0; i < PositionCount; ++i)
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
  FPointIndex const RootPoint = Points[0];
  FEdgeIndex const RootEdge = Kernel->MakeEdgePair();
  auto PreviousEdge = RootEdge;
  for (auto i = 1; i < Points.Num(); ++i)
  {
    FPointIndex const CurrentPoint = Points[i];
    FEdgeIndex const CurrentEdge = Kernel->MakeEdgePair();

    Kernel->ConnectEdges(PreviousEdge, CurrentPoint, CurrentEdge);

    PreviousEdge = CurrentEdge;
  }
  Kernel->ConnectEdges(PreviousEdge, RootPoint, RootEdge);
}

FFaceIndex UHedgeMesh::AddFace(
  FEdgeIndex const& RootEdge, TArray<FPointIndex> const& Points)
{
  auto const RootPoint = Edge(RootEdge).Adjacent().Vertex().Point().GetIndex();
  auto PreviousEdge = RootEdge;
  {
    auto const CurrentEdge = Kernel->MakeEdgePair();
    Kernel->ConnectEdges(PreviousEdge, RootPoint, CurrentEdge);
    PreviousEdge = CurrentEdge;
  }
  for (auto i = 0; i < Points.Num(); ++i)
  {
    auto const CurrentPoint = Points[i];
    auto const CurrentEdge = Kernel->MakeEdgePair();

    Kernel->ConnectEdges(PreviousEdge, CurrentPoint, CurrentEdge);

    PreviousEdge = CurrentEdge;
  }
  // TODO workout how to close this loop
  auto const CurrentPoint = Edge(RootEdge).Adjacent().Next().Vertex().Point().GetIndex();
  auto const CurrentEdge = Kernel->MakeEdgePair();
  Kernel->ConnectEdges(PreviousEdge, CurrentPoint, CurrentEdge);
}

FFaceIndex UHedgeMesh::AddFace(
  FEdgeIndex const& RootEdgeIndex, FPointIndex const& PointIndex)
{
  unimplemented();
  return FFaceIndex::Invalid;
}

FFaceIndex UHedgeMesh::AddFace(TArray<FEdgeIndex> const& Edges)
{
  unimplemented();
  return FFaceIndex::Invalid;
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
