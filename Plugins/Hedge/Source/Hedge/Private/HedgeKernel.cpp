// Copyright 2019 Chip Collier, Inc. All Rights Reserved.

#include "HedgeKernel.h"
#include "HedgeLogging.h"


FHalfEdge& UHedgeKernel::Get(FEdgeIndex Index)
{
  return Edges.Get(Index);
}

FFace& UHedgeKernel::Get(FFaceIndex Index)
{
  return Faces.Get(Index);
}

FVertex& UHedgeKernel::Get(FVertexIndex Index)
{
  return Vertices.Get(Index);
}

FPoint& UHedgeKernel::Get(FPointIndex Index)
{
  return Points.Get(Index);
}

FHalfEdge& UHedgeKernel::New(FEdgeIndex& OutIndex)
{
  return Edges.New(OutIndex);
}

FFace& UHedgeKernel::New(FFaceIndex& OutIndex)
{
  return Faces.New(OutIndex);
}

FVertex& UHedgeKernel::New(FVertexIndex& OutIndex)
{
  return Vertices.New(OutIndex);
}

FPoint& UHedgeKernel::New(FPointIndex& OutIndex)
{
  return Points.New(OutIndex);
}

FEdgeIndex UHedgeKernel::Add(FHalfEdge&& Edge)
{
  return Edges.Add(MoveTemp(Edge));
}

FFaceIndex UHedgeKernel::Add(FFace&& Face)
{
  return Faces.Add(MoveTemp(Face));
}

FVertexIndex UHedgeKernel::Add(FVertex&& Vertex)
{
  return Vertices.Add(MoveTemp(Vertex));
}

FPointIndex UHedgeKernel::Add(FPoint&& Point)
{
  return Points.Add(MoveTemp(Point));
}

void UHedgeKernel::Remove(FEdgeIndex Index)
{
  Edges.Remove(Index);
}

void UHedgeKernel::Remove(FFaceIndex Index)
{
  Faces.Remove(Index);
}

void UHedgeKernel::Remove(FVertexIndex Index)
{
  { 
    auto& Vertex = Get(Index);
    auto& Point = Get(Vertex.PointIndex);
    auto& Edge = Get(Vertex.EdgeIndex);
    Point.Vertices.Remove(Index);
    Edge.VertexIndex = FVertexIndex::Invalid;
  }

  Vertices.Remove(Index);
}

void UHedgeKernel::Remove(FPointIndex Index)
{
  {
    auto& Point = Points.Get(Index);
    for (auto const& VertexIndex : Point.Vertices)
    {
      auto& Vertex = Vertices.Get(VertexIndex);
      Vertex.PointIndex = FPointIndex::Invalid;
    }
  }
  Points.Remove(Index);
}

uint32 UHedgeKernel::NumPoints() const
{
  return Points.Num();
}

uint32 UHedgeKernel::NumVertices() const
{
  return Vertices.Num();
}

uint32 UHedgeKernel::NumFaces() const
{
  return Faces.Num();
}

uint32 UHedgeKernel::NumEdges() const
{
  return Edges.Num();
}

void UHedgeKernel::Defrag()
{
  ErrorLog("UHedgeKernel::Defrag is not yet implemented.");
  unimplemented();
}

FEdgeIndex UHedgeKernel::MakeEdgePair()
{
  FEdgeIndex EdgeIndex0;
  FHalfEdge& Edge0 = New(EdgeIndex0);
  FEdgeIndex EdgeIndex1;
  FHalfEdge& Edge1 = New(EdgeIndex1);

  Edge0.AdjacentEdgeIndex = EdgeIndex1;
  Edge1.AdjacentEdgeIndex = EdgeIndex0;

  return EdgeIndex0;
}

FFaceIndex UHedgeKernel::MakeFace(FEdgeIndex RootEdgeIndex)
{
  FFaceIndex FaceIndex;
  FFace& Face = New(FaceIndex);
  Face.RootEdgeIndex = RootEdgeIndex;

  int32 EdgeCount = 1;
  auto CurrentEdgeIndex = RootEdgeIndex;
  while (CurrentEdgeIndex)
  {
    auto& Edge = Get(CurrentEdgeIndex);
    Edge.FaceIndex = FaceIndex;

    check(Edge.NextEdgeIndex != CurrentEdgeIndex);
    if (Edge.NextEdgeIndex == RootEdgeIndex)
    {
      break;
    }

    CurrentEdgeIndex = Edge.NextEdgeIndex;
    ++EdgeCount;
  }

  if (EdgeCount > 3)
  {
    // TODO: Build triangle array.
  }

  return FaceIndex;
}

FVertexIndex UHedgeKernel::ConnectEdges(
  FEdgeIndex EdgeIndexA, FPointIndex PointIndex, FEdgeIndex EdgeIndexB)
{
  FHalfEdge& EdgeA = Get(EdgeIndexA);
  FHalfEdge& EdgeB = Get(EdgeIndexB);
  FPoint& Point = Get(PointIndex);

  FVertexIndex VertexIndex;
  FVertex& Vertex = New(VertexIndex);

  Vertex.PointIndex = PointIndex;
  Vertex.EdgeIndex = EdgeIndexB;

  EdgeA.NextEdgeIndex = EdgeIndexB;
  EdgeB.PrevEdgeIndex = EdgeIndexA;
  EdgeB.VertexIndex = VertexIndex;

  Point.Vertices.Add(VertexIndex);

  return VertexIndex;
}
