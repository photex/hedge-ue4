// Copyright 2019 Chip Collier. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HedgeTypes.h"
#include "HedgeMesh.generated.h"

struct FPxHalfEdge;
struct FPxFace;
struct FPxVertex;
struct FPxPoint;

class UHedgeKernel;
class UHedgeMesh;
class UHedgeMeshBuilder;
class UHedgeElementLoopBuilder;


/**
 * Very little information currently here but in the future
 * it can be exanded to include whatever information we need.
 *
 * @note This seemed like a good alternative to having many
 *       different methods to interrogate the internal state
 *       of the mesh.
 */
struct FHedgeMeshStats
{
  uint32 NumPoints;
  uint32 NumFaces;
  uint32 NumEdges;
  uint32 NumVertices;
};

/**
 * This is the "high level" interface for building and modifying hedge
 * meshes.
 *
 * Little by little this interface should grow to provide the most common
 * facilities needed for a modeling tool
 */
UCLASS()
class UHedgeMesh : public UObject
{
  GENERATED_BODY()

  UPROPERTY()
  UHedgeKernel* Kernel;

public:
  UHedgeMesh();

  void GetStats(FHedgeMeshStats& OutStats) const;

  FPxFace Face(FFaceIndex const& Index) const;
  FPxHalfEdge Edge(FEdgeIndex const& Index) const;
  FPxPoint Point(FPointIndex const& Index) const;
  FPxVertex Vertex(FVertexIndex const& Index) const;

  /**
   * Given an array of positions, new points are added to the mesh.
   */
  TArray<FPointIndex> AddPoints(TArray<FVector> const& Positions) const;
  /**
   * Given an array of positions, new points are added to the mesh.
   *
   * @note: This is the variant of this method that actually adds points to the mesh.
   *        In cases where you aren't building an array using TArray this is perhaps
   *        the more convenient option.
   */
  TArray<FPointIndex> AddPoints(FVector const Positions[], uint32 PositionCount) const;

  /**
   * Given an array of points; create all required mesh elements to form a face.
   *
   * @note: It is assumed that the points are specified in the correct winding order.
   */
  FFaceIndex AddFace(TArray<FPointIndex> const& Points);
  /**
   * Given an edge and an array of points; create all required mesh elements and create
   * a new face extending from the specified edge.
   *
   * @note: It is assumed that the points are specified in the correct winding order
   *        and that the specified edge is a suitable boundary edge to form the face.
   *        "suitable" here means that the adjacent half-edge of the specified edge
   *        has a valid vertex which has a valid point and the 'next' edge from there
   *        also has a valid point.
   */
  FFaceIndex AddFace(FEdgeIndex const& RootEdgeIndex, TArray<FPointIndex> const& Points);
  /**
   * Given an edge and an array of points; Create all required mesh elements to
   * create a new face (triangle) extending from the specified edge.
   *
   * @note: It is assumed that the specified edge is a suitable boundary edge to form the face.
   */
  FFaceIndex AddFace(FEdgeIndex const& RootEdgeIndex, FPointIndex const& PointIndex);
  /**
   * Given a list of edges; Connect each edge and create a new face.
   *
   * @note: It is assumed that the specified edges are "related" in such a way that they
   *        can be connected to form a face.
   */
  FFaceIndex AddFace(TArray<FEdgeIndex> const& Edges);

  /**
   * Removes the specified edge, and associated elements.
   *
   * If this edge is connected to a previous and next edge then
   * those vertices will be removed and the other edges updated.
   *
   * If this edge is not a boundary edge then the associated
   * face will be removed and all other edges updated.
   *
   * If this edge has a non-boundary adjacent edge then we apply
   * the dissolve to it as well.
   */
  void Dissolve(FEdgeIndex Index);

  /**
   * Removes the specified face and updates the associated edges.
   */
  void Dissolve(FFaceIndex Index);

  /**
   * Oh my
   */
  void Dissolve(FVertexIndex Index);

  /**
   * Nuke it from orbit why dontcha
   */
  void Dissolve(FPointIndex Index);
};
