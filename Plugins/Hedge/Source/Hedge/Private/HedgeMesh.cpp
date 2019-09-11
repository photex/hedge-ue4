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

UHedgeKernel* UHedgeMesh::GetKernel() const
{
  return Kernel;
}

FPxFace UHedgeMesh::Face(uint32 const Index) const
{
  return Face(FFaceHandle(Index));
}

FPxFace UHedgeMesh::Face(FFaceHandle const& Handle) const
{
  return FPxFace(Kernel, Handle);
}

UHedgeMesh::FFaceRangeIterator UHedgeMesh::Faces() const
{
  return FFaceRangeIterator(Kernel);
}

FPxHalfEdge UHedgeMesh::Edge(uint32 const Index) const
{
  return Edge(FEdgeHandle(Index));
}

FPxHalfEdge UHedgeMesh::Edge(FEdgeHandle const& Handle) const
{
  return FPxHalfEdge(Kernel, Handle);
}

UHedgeMesh::FHalfEdgeRangeIterator UHedgeMesh::Edges() const
{
  return FHalfEdgeRangeIterator(Kernel);
}

FPxPoint UHedgeMesh::Point(uint32 const Index) const
{
  return Point(FPointHandle(Index));
}

FPxPoint UHedgeMesh::Point(FPointHandle const& Handle) const
{
  return FPxPoint(Kernel, Handle);
}

UHedgeMesh::FPointRangeIterator UHedgeMesh::Points() const
{
  return FPointRangeIterator(Kernel);
}

FPxVertex UHedgeMesh::Vertex(uint32 const Index) const
{
  return Vertex(FVertexHandle(Index));
}

FPxVertex UHedgeMesh::Vertex(FVertexHandle const& Handle) const
{
  return FPxVertex(Kernel, Handle);
}

UHedgeMesh::FVertexRangeIterator UHedgeMesh::Vertices() const
{
  return FVertexRangeIterator(Kernel);
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
  uint32 PointCount = Points.Num();
  return AddFace(Points.GetData(), PointCount);
}

FFaceHandle UHedgeMesh::AddFace(FPointHandle const Points[], uint32 PointCount)
{
  if (PointCount < 3)
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
  for (uint32 i = 2; i < PointCount; ++i)
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
  uint32 const PointCount = Points.Num();
  if (PointCount < 1)
  {
    ErrorLog("Unable to add a new face to mesh without at least 1 point.");
    return FFaceHandle::Invalid;
  }
  FFaceHandle FaceHandle;
  FFace& Face = Kernel->New(FaceHandle);

  auto PreviousEdge = RootEdge;
  for (uint32 i = 0; i < PointCount; ++i)
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

FFaceHandle UHedgeMesh::AddFace(
  TArray<FEdgeHandle> const& Edges, TArray<FPointHandle> const& Points)
{
  unimplemented();
  return FFaceHandle::Invalid;
}

FFaceHandle UHedgeMesh::AddFace(TArray<FEdgeHandle> const& Edges)
{
  auto const EdgeCount = Edges.Num();
  return AddFace(Edges.GetData(), EdgeCount);
}

FFaceHandle UHedgeMesh::AddFace(FEdgeHandle const Edges[], uint32 EdgeCount)
{
  if (EdgeCount < 2)
  {
    ErrorLog("Unable to create a face without at least 2 edges.");
    return FFaceHandle::Invalid;
  }
  FFaceHandle FaceHandle;
  Kernel->New(FaceHandle);

  auto const RootEdge = Edges[0];
  auto PreviousEdge = RootEdge;
  for (uint32 i = 1; i < EdgeCount; ++i)
  {
    auto const CurrentEdge = Edges[i];
    Kernel->ConnectEdges(PreviousEdge, CurrentEdge);
    Kernel->Get(CurrentEdge).Face = FaceHandle;
    PreviousEdge = CurrentEdge;
  }

  // In a certain case, we need to make the last edge
  if (EdgeCount == 2)
  {
    PreviousEdge = Kernel->MakeEdgePair(PreviousEdge, RootEdge, FaceHandle);
  }

  Kernel->ConnectEdges(PreviousEdge, RootEdge);

  return FaceHandle;
}

FFaceHandle UHedgeMesh::AddFace(FEdgeHandle const& RootEdge)
{
  TArray<FEdgeHandle> Edges = {RootEdge};
  auto const GetNextEdge = [this, &RootEdge](FEdgeHandle CurrentEdge)
  {
    if (!CurrentEdge)
    {
      return FEdgeHandle::Invalid;
    }

    if (CurrentEdge == RootEdge)
    {
      return FEdgeHandle::Invalid;
    }

    if (!Kernel->IsValidHandle(CurrentEdge))
    {
      return FEdgeHandle::Invalid;
    }

    auto const NextEdge = Kernel->Get(CurrentEdge).NextEdge;
    if (CurrentEdge == NextEdge)
    {
      return FEdgeHandle::Invalid;
    }

    return NextEdge;
  };

  auto CurrentEdge = GetNextEdge(RootEdge);
  while(CurrentEdge)
  {
    Edges.Add(CurrentEdge);
    CurrentEdge = GetNextEdge(CurrentEdge);
  }

  unimplemented();
  return FFaceHandle::Invalid;
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
