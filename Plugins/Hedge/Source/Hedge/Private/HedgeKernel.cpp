// Copyright 2019 Chip Collier. All Rights Reserved.

#include "HedgeKernel.h"
#include "HedgeLogging.h"
#include "HedgeProxies.h"

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

  RemapElements(RemapData);
}

void UHedgeKernel::RemapElements(FRemapData const& RemapData)
{
  for (auto& Point : Points.Elements)
  {
    FVertexSet NewSet;
    for (auto VertexHandle : Point.Vertices)
    {
      NewSet.Add(RemapData.Vertices[VertexHandle.GetIndex()]);
    }
    check(NewSet.Num() == Point.Vertices.Num());
    Point.Vertices = MoveTemp(NewSet);
  }

  for (auto& Vertex : Vertices.Elements)
  {
    Vertex.Edge = RemapData.Edges[Vertex.Edge.GetIndex()];
    Vertex.Point = RemapData.Points[Vertex.Point.GetIndex()];
  }

  for (auto& Face : Faces.Elements)
  {
    Face.RootEdge = RemapData.Edges[Face.RootEdge.GetIndex()];
    for (auto& Triangle : Face.Triangles)
    {
      Triangle.V0 = RemapData.Vertices[Triangle.V0.GetIndex()];
      Triangle.V1 = RemapData.Vertices[Triangle.V1.GetIndex()];
      Triangle.V2 = RemapData.Vertices[Triangle.V2.GetIndex()];
    }
  }

  for (auto& Edge : Edges.Elements)
  {
    Edge.NextEdge = RemapData.Edges[Edge.NextEdge.GetIndex()];
    Edge.PrevEdge = RemapData.Edges[Edge.PrevEdge.GetIndex()];
    Edge.AdjacentEdge = RemapData.Edges[Edge.AdjacentEdge.GetIndex()];
    Edge.Vertex = RemapData.Vertices[Edge.Vertex.GetIndex()];
    Edge.Face = RemapData.Faces[Edge.Face.GetIndex()];
  }
}

FVertexHandle UHedgeKernel::MakeVertex(
  FPointHandle const PointHandle, 
  FEdgeHandle const EdgeHandle)
{
  FVertexHandle VertexHandle = Vertices.New();

  if (PointHandle)
  {
    SetVertexPoint(VertexHandle, PointHandle);
  }

  if (EdgeHandle)
  {
    SetVertexEdge(VertexHandle, EdgeHandle);
  }

  return MoveTemp(VertexHandle);
}

void UHedgeKernel::NewEdgePair(FEdgeHandle& OutEdge0, FEdgeHandle& OutEdge1)
{
  auto& Edge0 = New(OutEdge0);
  auto& Edge1 = New(OutEdge1);
  Edge0.AdjacentEdge = OutEdge1;
  Edge1.AdjacentEdge = OutEdge0;
}

FEdgeHandle UHedgeKernel::MakeEdgePair(
  FPointHandle const Point0Handle, 
  FPointHandle const Point1Handle, 
  FFaceHandle const FaceHandle)
{
  FEdgeHandle Edge0Handle;
  FEdgeHandle Edge1Handle;
  NewEdgePair(Edge0Handle, Edge1Handle);

  MakeVertex(Point0Handle, Edge0Handle);
  MakeVertex(Point1Handle, Edge1Handle);

  if (FaceHandle)
  {
    auto& Edge0 = Get(Edge0Handle);
    Edge0.Face = FaceHandle;
  }

  return Edge0Handle;
}

FEdgeHandle UHedgeKernel::MakeEdgePair(
  FEdgeHandle const PreviousEdgeHandle, 
  FPointHandle const PointHandle, 
  FFaceHandle const FaceHandle)
{
  auto& PreviousEdge = Get(PreviousEdgeHandle);
  auto const PreviousAdjacentVertexHandle = Get(PreviousEdge.AdjacentEdge).Vertex;
  auto const PreviousPointHandle = Get(PreviousAdjacentVertexHandle).Point;

  auto const NewEdgeHandle = MakeEdgePair(PreviousPointHandle, PointHandle, FaceHandle);
  ConnectEdges(PreviousEdgeHandle, NewEdgeHandle);

  return NewEdgeHandle;
}

FEdgeHandle UHedgeKernel::MakeEdgePair(
  FEdgeHandle const PreviousEdgeHandle, 
  FEdgeHandle const NextEdgeHandle, 
  FFaceHandle const FaceHandle)
{
  FPxHalfEdge const PreviousEdge(this, PreviousEdgeHandle);
  FPxHalfEdge const NextEdge(this, NextEdgeHandle);

  auto const P0 = PreviousEdge.Adjacent().Vertex().Point().GetHandle();
  auto const P1 = NextEdge.Vertex().Point().GetHandle();
  auto const NewEdgeHandle = MakeEdgePair(P0, P1, FaceHandle);

  ConnectEdges(PreviousEdgeHandle, NewEdgeHandle);
  ConnectEdges(NewEdgeHandle, NextEdgeHandle);

  return NewEdgeHandle;
}

FEdgeHandle UHedgeKernel::MakeEdgePair(FFaceHandle FaceHandle)
{
  FEdgeHandle E0;
  FEdgeHandle E1;
  NewEdgePair(E0, E1);

  auto const V0 = Vertices.New();
  auto const V1 = Vertices.New();

  SetVertexEdge(V0, E0);
  SetVertexEdge(V1, E1);

  if (FaceHandle)
  {
    auto& Edge0 = Get(E0);
    Edge0.Face = FaceHandle;
  }

  return E0;
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

void UHedgeKernel::ConnectEdges(FEdgeHandle const A, FEdgeHandle const B)
{
  FHalfEdge& EdgeA = Get(A);
  FHalfEdge& EdgeB = Get(B);

  EdgeA.NextEdge = B;
  EdgeB.PrevEdge = A;

  // Hrm... started thinking up some heuristics for also connecting
  // adjacent boundary edges but there are so many "edge" cases (LOLOLOL)
  // that it's clear to me this function should not be handling it.
}

void UHedgeKernel::SetVertexPoint(FVertexHandle const VertexHandle, FPointHandle const PointHandle)
{
  auto& Vert = Get(VertexHandle);
  auto& Point = Get(PointHandle);
  if (Vert.Point)
  {
    // Unset the previous point
    auto& PreviousPoint = Get(Vert.Point);
    PreviousPoint.Vertices.Remove(VertexHandle);
  }
  Vert.Point = PointHandle;
  Point.Vertices.Add(VertexHandle);
}

void UHedgeKernel::SetVertexEdge(FVertexHandle VertexHandle, FEdgeHandle EdgeHandle)
{
  auto& Vert = Get(VertexHandle);
  auto& Edge = Get(EdgeHandle);

  if (Vert.Edge)
  {
    auto& PreviousEdge = Get(Vert.Edge);
    PreviousEdge.Vertex = FVertexHandle::Invalid;
  }

  if (Edge.Vertex)
  {
    auto& FormerVert = Get(Edge.Vertex);
    FormerVert.Edge = FEdgeHandle::Invalid;
  }

  Vert.Edge = EdgeHandle;
  Edge.Vertex = VertexHandle;
}
