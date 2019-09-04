// Copyright 2019 Chip Collier. All Rights Reserved.

#include "HedgeKernel.h"
#include "HedgeLogging.h"


void UHedgeKernel::RemapPoints(FPointRemapTable const& Table)
{
  for (auto& Vertex : Vertices.Elements)
  {
    if (Vertex.PointIndex)
    {
      Vertex.PointIndex = Table[Vertex.PointIndex];
    }
  }
}

void UHedgeKernel::RemapEdges(FEdgeRemapTable const& Table)
{
  for (auto& Vertex : Vertices.Elements)
  {
    if (Vertex.EdgeIndex)
    {
      Vertex.EdgeIndex = Table[Vertex.EdgeIndex];
    }
  }
  for (auto& Edge : Edges.Elements)
  {
    if (Edge.NextEdgeIndex)
    {
      Edge.NextEdgeIndex = Table[Edge.NextEdgeIndex];
    }

    if (Edge.PrevEdgeIndex)
    {
      Edge.PrevEdgeIndex = Table[Edge.PrevEdgeIndex];
    }

    if (Edge.AdjacentEdgeIndex)
    {
      Edge.AdjacentEdgeIndex = Table[Edge.AdjacentEdgeIndex];
    }
  }
  for (auto& Face : Faces.Elements)
  {
    if (Face.RootEdgeIndex)
    {
      Face.RootEdgeIndex = Table[Face.RootEdgeIndex];
    }
  }
}

void UHedgeKernel::RemapFaces(FFaceRemapTable const& Table)
{
  for (auto& Edge : Edges.Elements)
  {
    if (Edge.FaceIndex)
    {
      Edge.FaceIndex = Table[Edge.FaceIndex];
    }
  }
}

void UHedgeKernel::RemapVertices(FVertexRemapTable const& Table)
{
  for (auto& Point : Points.Elements)
  {
    TSet<FVertexHandle> NewVertexSet;
    for (auto const& VertexIndex : Point.Vertices)
    {
      NewVertexSet.Add(Table[VertexIndex]);
    }
    Point.Vertices = MoveTemp(NewVertexSet);
  }

  for (auto& Edge : Edges.Elements)
  {
    if (Edge.VertexIndex)
    {
      Edge.VertexIndex = Table[Edge.VertexIndex];
    }
  }

  for (auto& Face : Faces.Elements)
  {
    for (auto& Triangle : Face.Triangles)
    {
      Triangle.VertexIndex0 = Table[Triangle.VertexIndex0];
      Triangle.VertexIndex1 = Table[Triangle.VertexIndex1];
      Triangle.VertexIndex2 = Table[Triangle.VertexIndex2];
    }
  }
}

bool UHedgeKernel::IsValidIndex(FEdgeIndex const Index) const
{
  return Edges.IsValidIndex(Index);
}

bool UHedgeKernel::IsValidIndex(FFaceHandle const Index) const
{
  return Faces.IsValidIndex(Index);
}

bool UHedgeKernel::IsValidIndex(FVertexHandle const Index) const
{
  return Vertices.IsValidIndex(Index);
}

bool UHedgeKernel::IsValidIndex(FPointHandle const Index) const
{
  return Points.IsValidIndex(Index);
}

FHalfEdge& UHedgeKernel::Get(FEdgeIndex const Index)
{
  return Edges.Get(Index);
}

FFace& UHedgeKernel::Get(FFaceHandle const Index)
{
  return Faces.Get(Index);
}

FVertex& UHedgeKernel::Get(FVertexHandle const Index)
{
  return Vertices.Get(Index);
}

FPoint& UHedgeKernel::Get(FPointHandle const Index)
{
  return Points.Get(Index);
}

FHalfEdge& UHedgeKernel::New(FEdgeIndex& OutIndex)
{
  OutIndex = Edges.New();
  return Get(OutIndex);
}

FFace& UHedgeKernel::New(FFaceHandle& OutIndex)
{
  OutIndex = Faces.New();
  return Get(OutIndex);
}

FVertex& UHedgeKernel::New(FVertexHandle& OutIndex)
{
  OutIndex = Vertices.New();
  return Get(OutIndex);
}

FPoint& UHedgeKernel::New(FPointHandle& OutIndex)
{
  OutIndex = Points.New();
  return Get(OutIndex);
}

FPoint& UHedgeKernel::New(FPointHandle& OutIndex, FVector Position)
{
  OutIndex = Points.New(Position);
  return Get(OutIndex);
}

FEdgeIndex UHedgeKernel::Add(FHalfEdge&& Edge)
{
  return Edges.Add(MoveTemp(Edge));
}

FFaceHandle UHedgeKernel::Add(FFace&& Face)
{
  return Faces.Add(MoveTemp(Face));
}

FVertexHandle UHedgeKernel::Add(FVertex&& Vertex)
{
  return Vertices.Add(MoveTemp(Vertex));
}

FPointHandle UHedgeKernel::Add(FPoint&& Point)
{
  return Points.Add(MoveTemp(Point));
}

void UHedgeKernel::Remove(FEdgeIndex const Index)
{
  if (!IsValidIndex(Index))
  {
    return;
  }

  { // Cleanup of any referring elements
    auto& Edge = Get(Index);

    if (IsValidIndex(Edge.VertexIndex))
    {
      bool bShouldRemoveVert = false;
      {
        auto& Vert = Get(Edge.VertexIndex);
        if (Vert.EdgeIndex == Index)
        {
          Vert.EdgeIndex = FEdgeIndex::Invalid;
          bShouldRemoveVert = true;
        }
      }
      if (bShouldRemoveVert)
      {
        Remove(Edge.VertexIndex);
      }
    }

    if (IsValidIndex(Edge.NextEdgeIndex))
    {
      auto& Next = Get(Edge.NextEdgeIndex);
      if (Next.PrevEdgeIndex == Index)
      {
        Next.PrevEdgeIndex = FEdgeIndex::Invalid;
      }
    }

    if (IsValidIndex(Edge.PrevEdgeIndex))
    {
      auto& Previous = Get(Edge.PrevEdgeIndex);
      if (Previous.NextEdgeIndex == Index)
      {
        Previous.NextEdgeIndex = FEdgeIndex::Invalid;
      }
    }

    // TODO: This raises some questions, but I think it's the right way to start
    if (IsValidIndex(Edge.AdjacentEdgeIndex))
    {
      { // First we cleanup the reference to this edge.
        auto& Adjacent = Get(Edge.AdjacentEdgeIndex);
        if (Adjacent.AdjacentEdgeIndex == Index)
        {
          Adjacent.AdjacentEdgeIndex = FEdgeIndex::Invalid;
        }
      }
      Remove(Edge.AdjacentEdgeIndex);
    }

    if (IsValidIndex(Edge.FaceIndex))
    {
      auto& Face = Get(Edge.FaceIndex);
      if (Face.RootEdgeIndex == Index)
      {
        if (IsValidIndex(Edge.NextEdgeIndex))
        {
          Face.RootEdgeIndex = Edge.NextEdgeIndex;
        }
        else if (IsValidIndex(Edge.PrevEdgeIndex))
        {
          Face.RootEdgeIndex = Edge.NextEdgeIndex;
        }
        else
        {
          // TODO: Determine whether it makes sense to also remove the face.
          Face.RootEdgeIndex = FEdgeIndex::Invalid;
        }
      }
    }
  }
  Edges.Remove(Index);
}

void UHedgeKernel::Remove(FFaceHandle const Index)
{
  if (!IsValidIndex(Index))
  {
    return;
  }

  { // Cleanup of any referring elements
    auto& Face = Get(Index);
    auto EIndex = Face.RootEdgeIndex;
    while(IsValidIndex(EIndex))
    {
      auto& Edge = Get(EIndex);
      Edge.FaceIndex = FFaceHandle::Invalid;
      EIndex = Edge.NextEdgeIndex;
    }
  }
  
  Faces.Remove(Index);
}

void UHedgeKernel::Remove(FVertexHandle const Index)
{
  if (!IsValidIndex(Index))
  {
    return;
  }

  { // Cleanup associated references
    auto& Vertex = Get(Index);
    if (IsValidIndex(Vertex.PointIndex))
    {
      auto& Point = Get(Vertex.PointIndex);
      Point.Vertices.Remove(Index);
    }

    if (IsValidIndex(Vertex.EdgeIndex))
    {
      auto& Edge = Get(Vertex.EdgeIndex);
      Edge.VertexIndex = FVertexHandle::Invalid;
    }
  }

  Vertices.Remove(Index);
}

void UHedgeKernel::Remove(FPointHandle const Index)
{
  if (!IsValidIndex(Index))
  {
    return;
  }

  {
    auto& Point = Points.Get(Index);
    for (auto const& VertexIndex : Point.Vertices)
    {
      if (!IsValidIndex(VertexIndex))
      {
        continue;
      }
      auto& Vertex = Vertices.Get(VertexIndex);
      Vertex.PointIndex = FPointHandle::Invalid;
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
  FRemapData RemapData;
  Points.Defrag(RemapData.Points);
  Vertices.Defrag(RemapData.Vertices);
  Faces.Defrag(RemapData.Faces);
  Edges.Defrag(RemapData.Edges);

  RemapEdges(RemapData.Edges);
  RemapFaces(RemapData.Faces);
  RemapVertices(RemapData.Vertices);
  RemapPoints(RemapData.Points);
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

FEdgeIndex UHedgeKernel::MakeEdgePair(FFaceHandle FaceIndex)
{
  FEdgeIndex EdgeIndex0;
  FHalfEdge& Edge0 = New(EdgeIndex0);
  FEdgeIndex EdgeIndex1;
  FHalfEdge& Edge1 = New(EdgeIndex1);

  Edge0.AdjacentEdgeIndex = EdgeIndex1;
  Edge0.FaceIndex = FaceIndex;
  Edge1.AdjacentEdgeIndex = EdgeIndex0;

  return EdgeIndex0;
}

void UHedgeKernel::SetFace(FFaceHandle FaceIndex, FEdgeIndex const RootEdgeIndex)
{
  auto& Face = New(FaceIndex);
  Face.RootEdgeIndex = RootEdgeIndex;

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
  }
}

FVertexHandle UHedgeKernel::ConnectEdges(
  FEdgeIndex const EdgeIndexA, 
  FPointHandle const PointIndex, 
  FEdgeIndex const EdgeIndexB)
{
  FHalfEdge& EdgeA = Get(EdgeIndexA);
  FHalfEdge& EdgeB = Get(EdgeIndexB);

  FVertexHandle VertexIndex;
  FVertex& Vertex = New(VertexIndex);

  Vertex.PointIndex = PointIndex;
  Vertex.EdgeIndex = EdgeIndexB;

  EdgeA.NextEdgeIndex = EdgeIndexB;
  EdgeB.PrevEdgeIndex = EdgeIndexA;
  EdgeB.VertexIndex = VertexIndex;

  if (IsValidIndex(PointIndex))
  {
    FPoint& Point = Get(PointIndex);
    Point.Vertices.Add(VertexIndex);
  }

  return VertexIndex;
}
