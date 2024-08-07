// Copyright 2019 Chip Collier. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HedgeTypes.h"

/**
 * Common fields used in every mesh element
 */
struct FMeshElement
{
  /// The Tag field can be used as a discriminator if
  /// a modifying process needs it.
  uint16 Tag = 0;
};

/**
 * Principle structure to encode the connectivity of a mesh.
 */
struct FHalfEdge : FMeshElement
{
  /// The vertex this edge starts from
  FVertexHandle Vertex = FVertexHandle::Invalid;
  /// Either the face that this edge contributes to
  /// or 'Invalid' for boundary edges.
  FFaceHandle Face = FFaceHandle::Invalid;
  /// The next edge in the loop that forms a face.
  FEdgeHandle NextEdge = FEdgeHandle::Invalid;
  /// The previous edge in the loop that forms a face.
  FEdgeHandle PrevEdge = FEdgeHandle::Invalid;
  /// The adjacent 'twin' half edge.
  FEdgeHandle AdjacentEdge = FEdgeHandle::Invalid;
};

/**
 * Faces are formed by a directed loop of edges and represent
 * a renderable element of a mesh.
 * Faces with greater than 3 vertices in their boundary maintain
 * the list of triangles that they must be comprised of to be rendered.
 */
struct FFace : FMeshElement
{
  /// The first edge of a loop that forms the face.
  FEdgeHandle RootEdge = FEdgeHandle::Invalid;
  /// A list of the triangles that compose this face.
  /// (Perhaps empty when the face itself is already a triangle)
  FHedgeTriangleArray Triangles;
};

/**
 * Encodes the 3 vertices (in counter-clockwise order) of a
 * sub-triangle of a given face.
 */
struct FFaceTriangle
{
  FVertexHandle V0 = FVertexHandle::Invalid;
  FVertexHandle V1 = FVertexHandle::Invalid;
  FVertexHandle V2 = FVertexHandle::Invalid;
};

/**
 * Vertices represent the connection of two edges.
 * Each vertex has an associated point which holds
 * attributes shared by all associated vertices.
 * @todo: We need to support arbitrary attributes.
 */
struct FVertex : FMeshElement
{
  /// The point which holds any relevant attributes.
  FPointHandle Point = FPointHandle::Invalid;
  /// The edge eminating from this vertex.
  FEdgeHandle Edge = FEdgeHandle::Invalid;
};

/**
 * Points are the structure which holds the common
 * vertex attribute 'position'.
 * Multiple vertices may be associated with a point.
 * @todo We need to support arbitrary attributes.
 */
struct FPoint : FMeshElement
{
  /// The location of this point.
  FVector Position;
  /// The associated vertices
  FVertexSet Vertices;

  FPoint(FVector InPosition)
    : Position(MoveTemp(InPosition))
  {
  }

  FPoint()
    : Position(FVector::ZeroVector)
  {
  }
};
