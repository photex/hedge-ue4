// Copyright 2019 Chip Collier. All Rights Reserved.

#include "HedgeKernel.h"
#include "HedgeLogging.h"


void UHedgeKernel::RemapPoints(FPointRemapTable const& Table)
{
  for (auto& Vertex : Vertices.Elements)
  {
    if (Vertex.Point)
    {
      Vertex.Point = Table[Vertex.Point];
    }
  }
}

void UHedgeKernel::RemapEdges(FEdgeRemapTable const& Table)
{
  for (auto& Vertex : Vertices.Elements)
  {
    if (Vertex.Edge)
    {
      Vertex.Edge = Table[Vertex.Edge];
    }
  }
  for (auto& Edge : Edges.Elements)
  {
    if (Edge.NextEdge)
    {
      Edge.NextEdge = Table[Edge.NextEdge];
    }

    if (Edge.PrevEdge)
    {
      Edge.PrevEdge = Table[Edge.PrevEdge];
    }

    if (Edge.AdjacentEdge)
    {
      Edge.AdjacentEdge = Table[Edge.AdjacentEdge];
    }
  }
  for (auto& Face : Faces.Elements)
  {
    if (Face.RootEdge)
    {
      Face.RootEdge = Table[Face.RootEdge];
    }
  }
}

void UHedgeKernel::RemapFaces(FFaceRemapTable const& Table)
{
  for (auto& Edge : Edges.Elements)
  {
    if (Edge.Face)
    {
      Edge.Face = Table[Edge.Face];
    }
  }
}

void UHedgeKernel::RemapVertices(FVertexRemapTable const& Table)
{
  for (auto& Point : Points.Elements)
  {
    TSet<FVertexHandle> NewVertexSet;
    for (auto const& VertexHandle : Point.Vertices)
    {
      NewVertexSet.Add(Table[VertexHandle]);
    }
    Point.Vertices = MoveTemp(NewVertexSet);
  }

  for (auto& Edge : Edges.Elements)
  {
    if (Edge.Vertex)
    {
      Edge.Vertex = Table[Edge.Vertex];
    }
  }

  for (auto& Face : Faces.Elements)
  {
    for (auto& Triangle : Face.Triangles)
    {
      Triangle.V0 = Table[Triangle.V0];
      Triangle.V1 = Table[Triangle.V1];
      Triangle.V2 = Table[Triangle.V2];
    }
  }
}

bool UHedgeKernel::IsValidHandle(FEdgeHandle const Handle) const
{
  return Edges.IsValidHandle(Handle);
}

bool UHedgeKernel::IsValidHandle(FFaceHandle const Handle) const
{
  return Faces.IsValidHandle(Handle);
}

bool UHedgeKernel::IsValidHandle(FVertexHandle const Handle) const
{
  return Vertices.IsValidHandle(Handle);
}

bool UHedgeKernel::IsValidHandle(FPointHandle const Handle) const
{
  return Points.IsValidHandle(Handle);
}

FHalfEdge& UHedgeKernel::Get(FEdgeHandle const Handle)
{
  return Edges.Get(Handle);
}

FFace& UHedgeKernel::Get(FFaceHandle const Handle)
{
  return Faces.Get(Handle);
}

FVertex& UHedgeKernel::Get(FVertexHandle const Handle)
{
  return Vertices.Get(Handle);
}

FPoint& UHedgeKernel::Get(FPointHandle const Handle)
{
  return Points.Get(Handle);
}

FHalfEdge& UHedgeKernel::New(FEdgeHandle& OutHandle)
{
  OutHandle = Edges.New();
  return Get(OutHandle);
}

FFace& UHedgeKernel::New(FFaceHandle& OutHandle)
{
  OutHandle = Faces.New();
  return Get(OutHandle);
}

FVertex& UHedgeKernel::New(FVertexHandle& OutHandle)
{
  OutHandle = Vertices.New();
  return Get(OutHandle);
}

FPoint& UHedgeKernel::New(FPointHandle& OutHandle)
{
  OutHandle = Points.New();
  return Get(OutHandle);
}

FPoint& UHedgeKernel::New(FPointHandle& OutHandle, FVector Position)
{
  OutHandle = Points.New(Position);
  return Get(OutHandle);
}

FEdgeHandle UHedgeKernel::Add(FHalfEdge&& Edge)
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

void UHedgeKernel::Remove(FEdgeHandle const Handle)
{
  if (!IsValidHandle(Handle))
  {
    return;
  }

  { // Cleanup of any referring elements
    auto& Edge = Get(Handle);

    if (IsValidHandle(Edge.Vertex))
    {
      bool bShouldRemoveVert = false;
      {
        auto& Vert = Get(Edge.Vertex);
        if (Vert.Edge == Handle)
        {
          Vert.Edge = FEdgeHandle::Invalid;
          bShouldRemoveVert = true;
        }
      }
      if (bShouldRemoveVert)
      {
        Remove(Edge.Vertex);
      }
    }

    if (IsValidHandle(Edge.NextEdge))
    {
      auto& Next = Get(Edge.NextEdge);
      if (Next.PrevEdge == Handle)
      {
        Next.PrevEdge = FEdgeHandle::Invalid;
      }
    }

    if (IsValidHandle(Edge.PrevEdge))
    {
      auto& Previous = Get(Edge.PrevEdge);
      if (Previous.NextEdge == Handle)
      {
        Previous.NextEdge = FEdgeHandle::Invalid;
      }
    }

    // TODO: This raises some questions, but I think it's the right way to start
    if (IsValidHandle(Edge.AdjacentEdge))
    {
      { // First we cleanup the reference to this edge.
        auto& Adjacent = Get(Edge.AdjacentEdge);
        if (Adjacent.AdjacentEdge == Handle)
        {
          Adjacent.AdjacentEdge = FEdgeHandle::Invalid;
        }
      }
      Remove(Edge.AdjacentEdge);
    }

    if (IsValidHandle(Edge.Face))
    {
      auto& Face = Get(Edge.Face);
      if (Face.RootEdge == Handle)
      {
        if (IsValidHandle(Edge.NextEdge))
        {
          Face.RootEdge = Edge.NextEdge;
        }
        else if (IsValidHandle(Edge.PrevEdge))
        {
          Face.RootEdge = Edge.NextEdge;
        }
        else
        {
          // TODO: Determine whether it makes sense to also remove the face.
          Face.RootEdge = FEdgeHandle::Invalid;
        }
      }
    }
  }
  Edges.Remove(Handle);
}

void UHedgeKernel::Remove(FFaceHandle const Handle)
{
  if (!IsValidHandle(Handle))
  {
    return;
  }

  { // Cleanup of any referring elements
    auto& Face = Get(Handle);
    auto CurrentEdgeHandle = Face.RootEdge;
    while(IsValidHandle(CurrentEdgeHandle))
    {
      auto& Edge = Get(CurrentEdgeHandle);
      Edge.Face = FFaceHandle::Invalid;
      CurrentEdgeHandle = Edge.NextEdge;
    }
  }
  
  Faces.Remove(Handle);
}

void UHedgeKernel::Remove(FVertexHandle const Handle)
{
  if (!IsValidHandle(Handle))
  {
    return;
  }

  { // Cleanup associated references
    auto& Vertex = Get(Handle);
    if (IsValidHandle(Vertex.Point))
    {
      auto& Point = Get(Vertex.Point);
      Point.Vertices.Remove(Handle);
    }

    if (IsValidHandle(Vertex.Edge))
    {
      auto& Edge = Get(Vertex.Edge);
      Edge.Vertex = FVertexHandle::Invalid;
    }
  }

  Vertices.Remove(Handle);
}

void UHedgeKernel::Remove(FPointHandle const Handle)
{
  if (!IsValidHandle(Handle))
  {
    return;
  }

  {
    auto& Point = Points.Get(Handle);
    for (auto const& VertexHandle : Point.Vertices)
    {
      if (!IsValidHandle(VertexHandle))
      {
        continue;
      }
      auto& Vertex = Vertices.Get(VertexHandle);
      Vertex.Point = FPointHandle::Invalid;
    }
  }
  Points.Remove(Handle);
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

FEdgeHandle UHedgeKernel::MakeEdgePair(FFaceHandle FaceHandle)
{
  FEdgeHandle Edge0Handle;
  FHalfEdge& Edge0 = New(Edge0Handle);
  FEdgeHandle Edge1Handle;
  FHalfEdge& Edge1 = New(Edge1Handle);

  Edge0.AdjacentEdge = Edge1Handle;
  Edge0.Face = FaceHandle;
  Edge1.AdjacentEdge = Edge0Handle;

  return Edge0Handle;
}

void UHedgeKernel::SetFace(FFaceHandle FaceHandle, FEdgeHandle const RootEdgeHandle)
{
  auto& Face = New(FaceHandle);
  Face.RootEdge = RootEdgeHandle;

  auto CurrentEdgeHandle = RootEdgeHandle;
  while (CurrentEdgeHandle)
  {
    auto& Edge = Get(CurrentEdgeHandle);
    Edge.Face = FaceHandle;

    check(Edge.NextEdge != CurrentEdgeHandle);
    if (Edge.NextEdge == RootEdgeHandle)
    {
      break;
    }

    CurrentEdgeHandle = Edge.NextEdge;
  }
}

FVertexHandle UHedgeKernel::ConnectEdges(
  FEdgeHandle const EdgeHandleA, 
  FPointHandle const PointHandle, 
  FEdgeHandle const EdgeHandleB)
{
  FHalfEdge& EdgeA = Get(EdgeHandleA);
  FHalfEdge& EdgeB = Get(EdgeHandleB);

  FVertexHandle VertexHandle;
  FVertex& Vertex = New(VertexHandle);

  Vertex.Point = PointHandle;
  Vertex.Edge = EdgeHandleB;

  EdgeA.NextEdge = EdgeHandleB;
  EdgeB.PrevEdge = EdgeHandleA;
  EdgeB.Vertex = VertexHandle;

  if (IsValidHandle(PointHandle))
  {
    FPoint& Point = Get(PointHandle);
    Point.Vertices.Add(VertexHandle);
  }

  return VertexHandle;
}
