// Copyright 2019 Chip Collier, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HedgeTypes.h"
#include "HedgeElements.h"
#include "HedgeKernel.generated.h"

/**
 * This is a very simple wrapper over TSparseArray used to enforce
 * strongly typed indices.
 *
 * @note In the rust and vanilla c++ versions I was ensuring that there
 *       was always a single 'inactive' element in the buffers. Because
 *       we aren't using a base collection type that doesn't require
 *       such a thing to function I can't see any real benefit to doing
 *       the same thing here.
 */
template<typename ElementType, typename ElementIndexType>
class THedgeElementBuffer
{
  TSparseArray<ElementType> Elements;

  friend class UHedgeKernel;

public:
  uint32 Num() const { return Elements.Num(); }
  FORCEINLINE void Reserve(uint32 const Count=0) { Elements.Reserve(Count); }
  FORCEINLINE void Reset(uint32 const Count=0)
  {
    Elements.Reset();
    Elements.Reserve(Count);
  }

  FORCEINLINE ElementIndexType Add()
  {
    auto Offset = Elements.Add(ElementType());
    return ElementIndexType(Offset);
  }

  FORCEINLINE ElementIndexType Add(ElementType&& Element)
  {
    auto Offset = Elements.Add(Element);
    return ElementIndexType(Offset);
  }

  FORCEINLINE ElementType& Get(ElementIndexType Index)
  {
    check(Elements.IsAllocated(Index.Offset()));
    return Elements[Index.Offset()];
  }

  FORCEINLINE void Remove(ElementIndexType Index)
  {
    check(Elements.IsAllocated(Index.Offset()));
    Elements.RemoveAtUninitialized(Index.Offset());
  }

  ElementType& New(ElementIndexType& OutIndex)
  {
    OutIndex = Add();
    return Get(OutIndex);
  }
};

/**
 * The mesh kernel contains element buffers and provides
 * fundamental utilities. It's meant to be low level and
 * is probably not fun to use.
 *
 * Unlike the other versions and attempts I've done so far,
 * this kernel is going to assume it's inputs are valid and
 * expects higher level code to have a plan for certain
 * externalities.
 */
UCLASS()
class UHedgeKernel final : public UObject
{
  GENERATED_BODY()
  THedgeElementBuffer<FHalfEdge, FEdgeIndex> Edges;
  THedgeElementBuffer<FVertex, FVertexIndex> Vertices;
  THedgeElementBuffer<FFace, FFaceIndex> Faces;
  THedgeElementBuffer<FPoint, FPointIndex> Points;
public:

  FHalfEdge& Get(FEdgeIndex Index);
  FFace& Get(FFaceIndex Index);
  FVertex& Get(FVertexIndex Index);
  FPoint& Get(FPointIndex Index);

  FHalfEdge& New(FEdgeIndex& OutIndex);
  FFace& New(FFaceIndex& OutIndex);
  FVertex& New(FVertexIndex& OutIndex);
  FPoint& New(FPointIndex& OutIndex);

  FEdgeIndex Add(FHalfEdge&& Edge);
  FFaceIndex Add(FFace&& Face);
  FVertexIndex Add(FVertex&& Vertex);
  FPointIndex Add(FPoint&& Point);

  void Remove(FEdgeIndex Index);
  void Remove(FFaceIndex Index);
  void Remove(FVertexIndex Index);
  void Remove(FPointIndex Index);

  uint32 NumPoints() const;
  uint32 NumVertices() const;
  uint32 NumFaces() const;
  uint32 NumEdges() const;

  /**
   * Sorts all buffers, moving any inactive elements to the back,
   * and updates all connectivity information.
   *
   * @warning Implementation pending.
   *
   * @note Compared to the rust version we've got some stuff to consider.
   *       The main difference of course is that in rust we are 'defrag'ing
   *       in-place and then shrinking the buffer.
   *       Because we're using a TSparseArray we *might* be able to do something
   *       similar, but looking at how it's done in the ue4 MeshDescription
   *       module, they are apparently copying/moving elements into a new
   *       sparse array and then providing a map from previous index to new index.
   *       This could be done here too and maybe it's just easier than
   *       the hoop jumping I do in the rust library.
   */
  void Defrag();

  /**
   * Create an empty edge and it's adjacent edge.
   * @returns The index of the first edge.
   */
  FEdgeIndex MakeEdgePair();

  /**
   * Create a new face, given a valid edge loop.
   * @todo: If the face has more than 3 sides, build it's triangle list.
   *
   * @param RootEdgeIndex: The first edge of a loop which forms the face
   * @returns The index of the newly created face.
   */
  FFaceIndex MakeFace(FEdgeIndex RootEdgeIndex);

  /**
   * Connect the two edges specified with a new vertex associated with
   * the specified point.
   *
   * (...)[EA] -> (V<P>)[EB] -> ...
   *
   * @param EdgeIndexA: Edge which should point to EdgeIndexB as 'next'
   * @param PointIndex: Point to associate with the new vertex
   * @param EdgeIndexB: Edge originating at the new vertex pointing to EdgeIndexA as 'previous'
   * @returns The index to the newly created vertex.
   */
  FVertexIndex ConnectEdges(
    FEdgeIndex EdgeIndexA, FPointIndex PointIndex, FEdgeIndex EdgeIndexB);
};
