

#include "HedgeProxies.h"
#include "HedgeKernel.h"
#include "HedgeElements.h"
#include "HedgeLogging.h"

FPxVertex FPxHalfEdge::Vertex() const
{
  auto& Edge = GetElement();
  return FPxVertex(Kernel, Edge.Vertex);
}

FPxFace FPxHalfEdge::Face() const
{
  auto& Edge = GetElement();
  return FPxFace(Kernel, Edge.Face);
}

FPxHalfEdge FPxHalfEdge::Next() const
{
  auto& Edge = GetElement();
  return FPxHalfEdge(Kernel, Edge.NextEdge);
}

FPxHalfEdge FPxHalfEdge::Prev() const
{
  auto& Edge = GetElement();
  return FPxHalfEdge(Kernel, Edge.PrevEdge);
}

FPxHalfEdge FPxHalfEdge::Adjacent() const
{
  auto& Edge = GetElement();
  return FPxHalfEdge(Kernel, Edge.AdjacentEdge);
}

bool FPxHalfEdge::IsBoundary() const
{
  auto& Edge = GetElement();
  if (!Edge.Face)
  {
    return true;
  }

  // *sigh*
  // If we just called 'IsBoundary' on the adjacent
  // proxy we recurse into the nothing.
  // Is this a .. bad .. design?
  auto& AdjacentEdge = Adjacent().GetElement();
  if (!AdjacentEdge.Face)
  {
    return true;
  }

  return false;
}

FHalfEdgePoints FPxHalfEdge::GetPoints() const
{
  FHalfEdgePoints Points;
  Points.Add(Vertex().Point());
  Points.Add(Next().Vertex().Point());
  return MoveTemp(Points);
}

FHalfEdgeVertices FPxHalfEdge::GetVertices() const
{
  FHalfEdgeVertices Vertices;
  Vertices.Add(Vertex());
  Vertices.Add(Next().Vertex());
  return MoveTemp(Vertices);
}

FPxHalfEdge FPxFace::RootEdge() const
{
  auto& Face = GetElement();
  return FPxHalfEdge(Kernel, Face.RootEdge);
}

TArray<FPxHalfEdge> FPxFace::GetPerimeterEdges() const
{
  TArray<FPxHalfEdge> Edges = { RootEdge() };

  auto const Root = Edges[0].GetHandle();
  auto CurrentEdge = Edges[0].Next();
  while(CurrentEdge.GetHandle() != Root)
  {
    Edges.Add(CurrentEdge);
    auto NextEdge = CurrentEdge.Next();
    if (NextEdge == CurrentEdge)
    {
      ErrorLogV("Edge %s is directly connected to itself!", 
        *(CurrentEdge.GetHandle()).ToString());
      break;
    }
    CurrentEdge = NextEdge;
  }

  return MoveTemp(Edges);
}

FPxHalfEdge FPxVertex::Edge() const
{
  auto& Vertex = GetElement();
  return FPxHalfEdge(Kernel, Vertex.Edge);
}

FPxPoint FPxVertex::Point() const
{
  auto& Vertex = GetElement();
  return FPxPoint(Kernel, Vertex.Point);
}

FVector FPxPoint::Position() const
{
  auto& Point = GetElement();
  return Point.Position;
}

void FPxPoint::SetPosition(FVector Position) const
{
  auto& Point = GetElement();
  Point.Position = MoveTemp(Position);
}

FVertexSet& FPxPoint::Vertices() const
{
  auto& Point = GetElement();
  return Point.Vertices;
}
