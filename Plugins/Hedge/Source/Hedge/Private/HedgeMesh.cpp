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

FPxFace UHedgeMesh::Face(FFaceHandle const& Handle) const
{
  return FPxFace(Kernel, Handle);
}

FPxHalfEdge UHedgeMesh::Edge(FEdgeHandle const& Handle) const
{
  return FPxHalfEdge(Kernel, Handle);
}

FPxPoint UHedgeMesh::Point(FPointHandle const& Handle) const
{
  return FPxPoint(Kernel, Handle);
}

FPxVertex UHedgeMesh::Vertex(FVertexHandle const& Handle) const
{
  return FPxVertex(Kernel, Handle);
}

TArray<FPointHandle> UHedgeMesh::AddPoints(
  TArray<FVector> const& Positions) const
{
  auto const PositionCount = Positions.Num();
  return AddPoints(Positions.GetData(), PositionCount);
}

TArray<FPointHandle> UHedgeMesh::AddPoints(
  FVector const Positions[], 
  uint32 const PositionCount) const
{
  TArray<FPointHandle> OutPointHandle;
  OutPointHandle.Reserve(PositionCount);
  for (uint32 i = 0; i < PositionCount; ++i)
  {
    auto PointHandle = Kernel->Add(FPoint(Positions[i]));
    OutPointHandle.Push(PointHandle);
  }
  return MoveTemp(OutPointHandle);
}

FFaceHandle UHedgeMesh::AddFace(TArray<FPointHandle> const& Points)
{
  if (Points.Num() < 3)
  {
    ErrorLog("Unable to add a new face to mesh without at least 3 points.");
    return FFaceHandle::Invalid;
  }
  FFaceHandle FaceHandle;
  FFace& Face = Kernel->New(FaceHandle);
  
  FPointHandle const RootPoint = Points[0];
  FEdgeHandle const RootEdge = Kernel->MakeEdgePair();
  auto PreviousEdge = RootEdge;
  for (auto i = 1; i < Points.Num(); ++i)
  {
    FPointHandle const CurrentPoint = Points[i];
    FEdgeHandle const CurrentEdge = Kernel->MakeEdgePair(FaceHandle);

    Kernel->ConnectEdges(PreviousEdge, CurrentPoint, CurrentEdge);

    PreviousEdge = CurrentEdge;
  }
  Kernel->ConnectEdges(PreviousEdge, RootPoint, RootEdge);
  Face.RootEdge = RootEdge;
  return FaceHandle;
}

FFaceHandle UHedgeMesh::AddFace(
  FEdgeHandle const& RootEdge, TArray<FPointHandle> const& Points)
{
  if (Points.Num() < 1)
  {
    ErrorLog("Unable to add a new face to mesh without at least 1 point.");
    return FFaceHandle::Invalid;
  }
  FFaceHandle FaceHandle;
  FFace& Face = Kernel->New(FaceHandle);

  auto const RootPoint = Edge(RootEdge).Adjacent().Vertex().Point().GetHandle();
  auto CurrentEdge = RootEdge;
  auto CurrentPoint = RootPoint;
  auto PreviousEdge = RootEdge;
  auto PreviousPoint = RootPoint;
  for (auto i = 0; i < Points.Num(); ++i)
  {
    CurrentPoint = Points[i];
    CurrentEdge = Kernel->MakeEdgePair(FaceHandle);
    Kernel->ConnectEdges(PreviousEdge, PreviousPoint, CurrentEdge);

    PreviousEdge = CurrentEdge;
    PreviousPoint = CurrentPoint;
  }
  CurrentPoint = Edge(RootEdge).Adjacent().Next().Vertex().Point().GetHandle();
  CurrentEdge = Kernel->MakeEdgePair(FaceHandle);
  Kernel->ConnectEdges(PreviousEdge, PreviousPoint, CurrentEdge);
  Kernel->ConnectEdges(CurrentEdge, CurrentPoint, RootEdge);
  Face.RootEdge = RootEdge;
  return FaceHandle;
}

FFaceHandle UHedgeMesh::AddFace(
  FEdgeHandle const& E0, FPointHandle const& P1)
{
  FFaceHandle FaceHandle;
  FFace& Face = Kernel->New(FaceHandle);
  Face.RootEdge = E0;

  auto& Edge0 = Kernel->Get(E0);
  Edge0.Face = FaceHandle;

  auto const P0 = Edge(E0).Adjacent().Vertex().Point().GetHandle();
  auto const P2 = Edge(E0).Adjacent().Next().Vertex().Point().GetHandle();

  auto const E1 = Kernel->MakeEdgePair(FaceHandle);
  auto const E2 = Kernel->MakeEdgePair(FaceHandle);

  Kernel->ConnectEdges(E0, P0, E1);
  Kernel->ConnectEdges(E1, P1, E2);
  Kernel->ConnectEdges(E2, P2, E0);

  return FaceHandle;
}

FFaceHandle UHedgeMesh::AddFace(TArray<FEdgeHandle> const& Edges)
{
  if (Edges.Num() < 3)
  {
    ErrorLog("Unable to create a face without at least 3 edges.");
    return FFaceHandle::Invalid;
  }
  FFaceHandle FaceHandle;
  FFace& Face = Kernel->New(FaceHandle);

  auto const RootEdge = Edges[0];
  auto LastEdge = RootEdge;
  auto CurrentEdge = LastEdge;
  for (auto i = 1; i < Edges.Num(); ++i)
  {
    CurrentEdge = Edges[i];
    auto const Point = Edge(LastEdge).Adjacent().Vertex().Point().GetHandle();
    Kernel->ConnectEdges(LastEdge, Point, CurrentEdge);
    Kernel->Get(CurrentEdge).Face = FaceHandle;
    LastEdge = CurrentEdge;
  }
  auto const Point = Edge(LastEdge).Adjacent().Vertex().Point().GetHandle();
  Kernel->ConnectEdges(LastEdge, Point, RootEdge);
  
  return FaceHandle;
}

void UHedgeMesh::Dissolve(FEdgeHandle Handle)
{
  unimplemented();
}

void UHedgeMesh::Dissolve(FFaceHandle Handle)
{
  unimplemented();
}

void UHedgeMesh::Dissolve(FVertexHandle Handle)
{
  unimplemented();
}

void UHedgeMesh::Dissolve(FPointHandle Handle)
{
  unimplemented();
}
