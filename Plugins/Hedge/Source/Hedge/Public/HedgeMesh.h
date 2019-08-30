// Copyright 2019 Chip Collier, Inc. All Rights Reserved.

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
 * //
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
