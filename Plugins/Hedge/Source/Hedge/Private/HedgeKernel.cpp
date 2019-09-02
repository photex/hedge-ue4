// Copyright 2019 Chip Collier. All Rights Reserved.

#include "HedgeKernel.h"
#include "HedgeLogging.h"


bool UHedgeKernel::IsValidIndex(FEdgeIndex const Index) const
{
  return Edges.Elements.IsAllocated(Index.Offset());
}

bool UHedgeKernel::IsValidIndex(FFaceIndex const Index) const
{
  return Faces.Elements.IsAllocated(Index.Offset());
}

bool UHedgeKernel::IsValidIndex(FVertexIndex const Index) const
{
  return Vertices.Elements.IsAllocated(Index.Offset());
}

bool UHedgeKernel::IsValidIndex(FPointIndex const Index) const
{
  return Points.Elements.IsAllocated(Index.Offset());
}

FHalfEdge& UHedgeKernel::Get(FEdgeIndex const Index)
{
  return Edges.Get(Index);
}

FFace& UHedgeKernel::Get(FFaceIndex const Index)
{
  return Faces.Get(Index);
}

FVertex& UHedgeKernel::Get(FVertexIndex const Index)
{
  return Vertices.Get(Index);
}

FPoint& UHedgeKernel::Get(FPointIndex const Index)
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

void UHedgeKernel::Remove(FEdgeIndex const Index)
{
  if (!IsValidIndex(Index))
  {
    return;
  }

  { // Cleanup of any referring elements
    auto& Edge = Get(Index);

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

void UHedgeKernel::Remove(FFaceIndex const Index)
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
      Edge.FaceIndex = FFaceIndex::Invalid;
      EIndex = Edge.NextEdgeIndex;
    }
  }
  
  Faces.Remove(Index);
}

void UHedgeKernel::Remove(FVertexIndex const Index)
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
      Edge.VertexIndex = FVertexIndex::Invalid;
    }
  }

  Vertices.Remove(Index);
}

void UHedgeKernel::Remove(FPointIndex const Index)
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
  //////////////////////
  /// Points
  
  FRemapTable<FPointIndex> PointRemapTable;
  Points.Defrag(PointRemapTable);
  for (auto& Vertex : Vertices.Elements)
  {
    if (Vertex.PointIndex)
    {
      Vertex.PointIndex = PointRemapTable[Vertex.PointIndex];
    }
  }

  //////////////////////
  /// Vertices

  FRemapTable<FVertexIndex> VertexRemapTable;
  Vertices.Defrag(VertexRemapTable);
  for (auto& Point : Points.Elements)
  {
    TSet<FVertexIndex> NewVertexSet;
    for (auto const& VertexIndex : Point.Vertices)
    {
      NewVertexSet.Add(VertexRemapTable[VertexIndex]);
    }
    Point.Vertices = MoveTemp(NewVertexSet);
  }
  for (auto& Edge : Edges.Elements)
  {
    if (Edge.VertexIndex)
    {
      Edge.VertexIndex = VertexRemapTable[Edge.VertexIndex];
    }
  }
  for (auto& Face : Faces.Elements)
  {
    for (auto& Triangle : Face.Triangles)
    {
      Triangle.VertexIndex0 = VertexRemapTable[Triangle.VertexIndex0];
      Triangle.VertexIndex1 = VertexRemapTable[Triangle.VertexIndex1];
      Triangle.VertexIndex2 = VertexRemapTable[Triangle.VertexIndex2];
    }
  }

  ///////////////////////
  /// Edges

  FRemapTable<FEdgeIndex> EdgeRemapTable;
  Edges.Defrag(EdgeRemapTable);
  for (auto& Vertex : Vertices.Elements)
  {
    if (Vertex.EdgeIndex)
    {
      Vertex.EdgeIndex = EdgeRemapTable[Vertex.EdgeIndex];
    }
  }
  for (auto& Edge : Edges.Elements)
  {
    if (Edge.NextEdgeIndex)
    {
      Edge.NextEdgeIndex = EdgeRemapTable[Edge.NextEdgeIndex];
    }

    if (Edge.PrevEdgeIndex)
    {
      Edge.PrevEdgeIndex = EdgeRemapTable[Edge.PrevEdgeIndex];
    }

    if (Edge.AdjacentEdgeIndex)
    {
      Edge.AdjacentEdgeIndex = EdgeRemapTable[Edge.AdjacentEdgeIndex];
    }
  }
  for (auto& Face : Faces.Elements)
  {
    if (Face.RootEdgeIndex)
    {
      Face.RootEdgeIndex = EdgeRemapTable[Face.RootEdgeIndex];
    }
  }

  ///////////////////////
  /// Faces
  
  FRemapTable<FFaceIndex> FaceRemapTable;
  Faces.Defrag(FaceRemapTable);
  for (auto& Edge : Edges.Elements)
  {
    if (Edge.FaceIndex)
    {
      Edge.FaceIndex = FaceRemapTable[Edge.FaceIndex];
    }
  }
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

FEdgeIndex UHedgeKernel::MakeEdgePair(FFaceIndex FaceIndex)
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

FFaceIndex UHedgeKernel::MakeFace(FEdgeIndex const RootEdgeIndex)
{
  FFaceIndex FaceIndex;
  FFace& Face = New(FaceIndex);
  Face.RootEdgeIndex = RootEdgeIndex;

  TArray<FVertexIndex> PerimeterVertices;
  int32 EdgeCount = 1;
  auto CurrentEdgeIndex = RootEdgeIndex;
  while (CurrentEdgeIndex)
  {
    auto& Edge = Get(CurrentEdgeIndex);
    Edge.FaceIndex = FaceIndex;
    PerimeterVertices.Add(Edge.VertexIndex);

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
  else if (EdgeCount == 3)
  {
    FFaceTriangle Triangle;
    Triangle.VertexIndex0 = PerimeterVertices[0];
    Triangle.VertexIndex1 = PerimeterVertices[1];
    Triangle.VertexIndex2 = PerimeterVertices[2];
    Face.Triangles.Add(MoveTemp(Triangle));
  }

  return FaceIndex;
}

FVertexIndex UHedgeKernel::ConnectEdges(
  FEdgeIndex const EdgeIndexA, 
  FPointIndex const PointIndex, 
  FEdgeIndex const EdgeIndexB)
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
