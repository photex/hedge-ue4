// Copyright 2019 Chip Collier, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HedgeKernel.generated.h"

struct FHalfEdge;
struct FFace;
struct FFaceTriangle;
struct FVertex;
struct FPoint;

struct FEdgeIndex;
struct FFaceIndex;
struct FVertexIndex;
struct FPointIndex;

//////////////////////////////////////////////////////////////
/// The mesh kernel contains element buffers and provides
/// the fundamental utilities. It's meant to be low level and
/// is probably not fun to use.

UCLASS()
class UHedgeKernel final : public UObject
{
  GENERATED_BODY()

  FHalfEdge* Get(FEdgeIndex Index);
  FFace* Get(FFaceIndex Index);
  FVertex* Get(FVertexIndex Index);
  FPoint* Get(FPointIndex Index);

  FHalfEdge* New(FEdgeIndex& OutIndex);
  FFace* New(FFaceIndex& OutIndex);
  FVertex* New(FVertexIndex& OutIndex);
  FPoint* New(FVector Pos, FPointIndex& OutIndex);

  FEdgeIndex Insert(FHalfEdge Edge);
  FFaceIndex Insert(FFace Face);
  FVertexIndex Insert(FVertex Vertex);
  FPointIndex Insert(FPoint Point);

  void Remove(FEdgeIndex Index);
  void Remove(FFaceIndex Index);
  void Remove(FVertexIndex Index);
  void Remove(FPointIndex Index);

  uint32 PointCount() const;
  uint32 VertexCount() const;
  uint32 FaceCount() const;
  uint32 EdgeCount() const;

  /**
   * Sorts all buffers, moving any inactive elements to the back,
   * and updates all connectivity information.
   */
  void Defrag();

  /**
   * Create an empty edge and it's adjacent edge.
   * @returns The index of the first edge.
   */
  FEdgeIndex MakeEdgePair();

  /**
   * Create a new face, given a valid edge loop. If the face has more
   * than 3 sides it will also build it's triangle list.
   *
   * @param RootEdgeIndex: The first edge of a loop which forms the face
   * @returns The index of the newly created face.
   */
  FFaceIndex MakeFace(FEdgeIndex RootEdgeIndex);

  /**
   * Connect the two edges specified with a new vertex associated with
   * the specified point.
   * @param EdgeIndexA: Edge which points toward the new vertex
   * @param PointIndex: Point to associate with the new vertex
   * @param EdgeIndexB: Edge which points out from the new vertex
   */
  FVertexIndex ConnectEdges(
    FEdgeIndex EdgeIndexA, FPointIndex PointIndex, FEdgeIndex EdgeIndexB);
};
