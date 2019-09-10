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

FPxFace UHedgeMesh::Face(uint32 const Index) const
{
  return Face(FFaceHandle(Index));
}

FPxFace UHedgeMesh::Face(FFaceHandle const& Handle) const
{
  return FPxFace(Kernel, Handle);
}

FPxHalfEdge UHedgeMesh::Edge(uint32 const Index) const
{
  return Edge(FEdgeHandle(Index));
}

FPxHalfEdge UHedgeMesh::Edge(FEdgeHandle const& Handle) const
{
  return FPxHalfEdge(Kernel, Handle);
}

FPxPoint UHedgeMesh::Point(uint32 const Index) const
{
  return Point(FPointHandle(Index));
}

FPxPoint UHedgeMesh::Point(FPointHandle const& Handle) const
{
  return FPxPoint(Kernel, Handle);
}

FPxVertex UHedgeMesh::Vertex(uint32 const Index) const
{
  return Vertex(FVertexHandle(Index));
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
  FPointHandle CurrentPoint = Points[1];
  FEdgeHandle const RootEdge = Kernel->MakeEdgePair(RootPoint, CurrentPoint, FaceHandle);
  auto PreviousEdge = RootEdge;
  for (auto i = 2; i < Points.Num(); ++i)
  {
    CurrentPoint = Points[i];
    FEdgeHandle const CurrentEdge = Kernel->MakeEdgePair(PreviousEdge, CurrentPoint, FaceHandle);

    PreviousEdge = CurrentEdge;
  }
  Kernel->MakeEdgePair(PreviousEdge, RootPoint, FaceHandle);
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

  auto PreviousEdge = RootEdge;
  for (auto i = 0; i < Points.Num(); ++i)
  {
    auto const CurrentPoint = Points[i];
    PreviousEdge = Kernel->MakeEdgePair(PreviousEdge, CurrentPoint, FaceHandle);
  }
  Kernel->MakeEdgePair(PreviousEdge, RootEdge, FaceHandle);
  Face.RootEdge = RootEdge;
  return FaceHandle;
}

FFaceHandle UHedgeMesh::AddFace(
  FEdgeHandle const& RootEdgeHandle, 
  FPointHandle const& PointHandle)
{
  FFaceHandle FaceHandle;
  FFace& Face = Kernel->New(FaceHandle);
  Face.RootEdge = RootEdgeHandle;

  auto& Edge0 = Kernel->Get(RootEdgeHandle);
  Edge0.Face = FaceHandle;

  auto const Edge1 = Kernel->MakeEdgePair(RootEdgeHandle, PointHandle, FaceHandle);
  Kernel->MakeEdgePair(Edge1, RootEdgeHandle, FaceHandle);

  return FaceHandle;
}

FFaceHandle UHedgeMesh::AddFace(FEdgeHandle const& RootEdgeHandle, FVector const Position)
{
  FPointHandle PointHandle;
  Kernel->New(PointHandle, Position);
  return AddFace(RootEdgeHandle, PointHandle);
}

FFaceHandle UHedgeMesh::AddFace(TArray<FEdgeHandle> const& Edges)
{
  if (Edges.Num() < 3)
  {
    ErrorLog("Unable to create a face without at least 3 edges.");
    return FFaceHandle::Invalid;
  }
  FFaceHandle FaceHandle;
  Kernel->New(FaceHandle);

  auto const RootEdge = Edges[0];
  auto PreviousEdge = RootEdge;
  for (auto i = 1; i < Edges.Num(); ++i)
  {
    auto const CurrentEdge = Edges[i];
    Kernel->ConnectEdges(PreviousEdge, CurrentEdge);
    Kernel->Get(CurrentEdge).Face = FaceHandle;
    PreviousEdge = CurrentEdge;
  }
  Kernel->ConnectEdges(PreviousEdge, RootEdge);
  
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
