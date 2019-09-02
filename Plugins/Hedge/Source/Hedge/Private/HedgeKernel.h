// Copyright 2019 Chip Collier. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HedgeTypes.h"
#include "HedgeElements.h"
#include "HedgeKernel.generated.h"

template<typename ElementIndexType>
using FRemapTable = TMap<ElementIndexType, ElementIndexType>;

/**
 * This is a very simple wrapper over TSparseArray used to enforce
 * strongly typed indices.
 *
 * @note In the rust and vanilla c++ versions I was ensuring that there
 *       was always a single 'inactive' element in the buffers. I can't
 *       see any real benefit to doing the same thing here (for now).
 *       The MeshDescription module has a much more fully realized version
 *       of this, and as our index type has essentially turned into a copy
 *       of the element id types of that module, I believe this will end
 *       up getting deprecated along with the our index type.
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

  // Using the same approach as the MeshDescription module because that
  // is just a heck of a lot easier than the stuff I did before when
  // trying to just reuse the container and sort/swap elements around.
  void Defrag(FRemapTable<ElementIndexType>& OutRemapTable)
  {
    OutRemapTable.Empty(Elements.GetMaxIndex());
    TSparseArray<ElementType> NewBuffer;
    for (typename TSparseArray<ElementType>::TIterator It( Elements ); It; ++It)
    {
      uint32 const PreviousOffset = It.GetIndex();
      uint32 const NewOffset = NewBuffer.Add(MoveTemp(*It));
      OutRemapTable.Add(ElementIndexType(PreviousOffset), ElementIndexType(NewOffset));
    }
    Elements = MoveTemp(NewBuffer);
  }
};

/**
 * The mesh kernel contains element buffers and provides
 * fundamental utilities. It's meant to be low level and
 * is probably not fun to use.
 *
 * The primary job of the kernel is to make sure we have a
 * consistent way to create and remove mesh elements with
 * correct connectivity. This is not always easy because
 * most things happen in steps and data is circularly associated.
 * Hopefully the API here makes it obvious or straight forward
 * to perform the most essential modifications to a mesh.
 *
 * @note Unlike the other versions and attempts I've done so far,
 *       this kernel is going to assume it's inputs are valid and
 *       expects higher level code to have a plan for certain
 *       externalities.
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

  bool IsValidIndex(FEdgeIndex Index) const;
  bool IsValidIndex(FFaceIndex Index) const;
  bool IsValidIndex(FVertexIndex Index) const;
  bool IsValidIndex(FPointIndex Index) const;

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
   * Reorganize all element buffers into contiguous arrays
   * and updates indices on related elements.
   */
  void Defrag();

  /**
   * Create an empty edge and it's adjacent edge.
   * @returns The index of the first edge.
   */
  FEdgeIndex MakeEdgePair();

  /**
   * Create an empty edge and it's adjacent edge. Associate
   * the first edge with the specified face.
   * @returns The index of the first edge.
   */
  FEdgeIndex MakeEdgePair(FFaceIndex FaceIndex);

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
