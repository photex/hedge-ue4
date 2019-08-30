

#include "HedgeProxies.h"
#include "HedgeKernel.h"
#include "HedgeElements.h"

FPxVertex FPxHalfEdge::Vertex() const
{
  auto& Edge = GetElement();
  return FPxVertex(Kernel, Edge.VertexIndex);
}

FPxFace FPxHalfEdge::Face() const
{
  auto& Edge = GetElement();
  return FPxFace(Kernel, Edge.FaceIndex);
}

FPxHalfEdge FPxHalfEdge::Next() const
{
  auto& Edge = GetElement();
  return FPxHalfEdge(Kernel, Edge.NextEdgeIndex);
}

FPxHalfEdge FPxHalfEdge::Prev() const
{
  auto& Edge = GetElement();
  return FPxHalfEdge(Kernel, Edge.PrevEdgeIndex);
}

FPxHalfEdge FPxHalfEdge::Adjacent() const
{
  auto& Edge = GetElement();
  return FPxHalfEdge(Kernel, Edge.AdjacentEdgeIndex);
}

bool FPxHalfEdge::IsBoundary() const
{
  auto& Edge = GetElement();
  if (!Edge.FaceIndex)
  {
    return true;
  }

  // *sigh*
  // If we just called 'IsBoundary' on the adjacent
  // proxy we recurse into the nothing.
  // Is this a .. bad .. design?
  auto& AdjacentEdge = Adjacent().GetElement();
  if (!AdjacentEdge.FaceIndex)
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
  return FPxHalfEdge(Kernel, Face.RootEdgeIndex);
}

FPxHalfEdge FPxVertex::Edge() const
{
  auto& Vertex = GetElement();
  return FPxHalfEdge(Kernel, Vertex.EdgeIndex);
}

FPxPoint FPxVertex::Point() const
{
  auto& Vertex = GetElement();
  return FPxPoint(Kernel, Vertex.PointIndex);
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

FVertexIndexSet& FPxPoint::Vertices() const
{
  auto& Point = GetElement();
  return Point.Vertices;
}
