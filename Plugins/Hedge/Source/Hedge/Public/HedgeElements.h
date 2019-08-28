// Copyright 2019 Chip Collier, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HedgeTypes.h"
#include "HedgeElements.generated.h"


UENUM()
enum class EMeshElementStatus : uint16
{
  /// This element is actively contributing to the topology of a mesh.
  Active = 0x0000,
  /// This element is available for re-use or removal.
  Inactive = 0x8000,
};

/**
 * Common fields used in every mesh element
 */
USTRUCT(BlueprintType)
struct FMeshElement
{
  GENERATED_BODY()

  /// Elements that are inactive no longer contribute to the mesh
  /// and the cells they occupy can be re-used or collected.
  EMeshElementStatus Status = EMeshElementStatus::Active;
  /// The Tag field can be used generically as a discriminator if
  /// a modifying process needs it.
  uint16 Tag = 0;
  /// Using 1 as the default so that 0 can have another meaning (ignored).
  FGeneration Generation = 1;
};

/**
 * Principle structure to encode the connectivity of a mesh.
 */
USTRUCT(BlueprintType)
struct FHalfEdge : public FMeshElement
{
  GENERATED_BODY()
  /// The vertex this edge starts from
  FVertexIndex VertexIndex;
  /// Either the face that this edge contributes to
  /// or 'Invalid' for boundary edges.
  FFaceIndex FaceIndex;
  /// The next edge in the loop that forms a face.
  FEdgeIndex NextEdgeIndex;
  /// The previous edge in the loop that forms a face.
  FEdgeIndex PrevEdgeIndex;
  /// The adjacent 'twin' half edge.
  FEdgeIndex AdjacentEdgeIndex;
};

/**
 * Faces are formed by a directed loop of edges and represent
 * a renderable element of a mesh.
 * Faces with greater than 3 vertices in their boundary maintain
 * the list of triangles that they must be comprised of to be rendered.
 */
USTRUCT(BlueprintType)
struct FFace : public FMeshElement
{
  GENERATED_BODY()
  /// The first edge of a loop that forms the face.
  FEdgeIndex RootEdgeIndex;
  /// A list of the triangles that compose this face.
  /// (Perhaps empty when the face itself is already a triangle)
  FHedgeFaceTriangleArray Triangles;
};

/**
 * Encodes the 3 vertices (in counter clockwise order) of a
 * sub-triangle of a given face.
 */
USTRUCT(BlueprintType)
struct FFaceTriangle
{
  GENERATED_BODY()
  FVertexIndex VertexIndex0;
  FVertexIndex VertexIndex1;
  FVertexIndex VertexIndex2;
};

/**
 * Vertices represent the connection of two edges.
 * Each vertex has an associated point which holds
 * attributes shared by all associated vertices.
 */
USTRUCT(BlueprintType)
struct FVertex : public FMeshElement
{
  GENERATED_BODY()
  /// The point which holds any relevant attributes.
  FPointIndex PointIndex;
  /// The edge eminating from this vertex.
  FEdgeIndex EdgeIndex;
};

/**
 * Points are the structure which holds the common
 * vertex attribute 'position'.
 * Multiple vertices may be associated with a point.
 * @todo There needs to be a general attribute system in place.
 */
USTRUCT(BlueprintType)
struct FPoint : public FMeshElement
{
  GENERATED_BODY()
  /// The location of this point.
  FPosition Position;
  /// The associated vertices
  FHedgeVertexArray Vertices;
};