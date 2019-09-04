// Copyright 2019 Chip Collier. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HedgeTypes.h"
#include "HedgeElements.h"
#include "HedgeKernel.generated.h"

template<typename ElementIndexType>
using TRemapTable = TMap<ElementIndexType, ElementIndexType>;

using FPointRemapTable = TRemapTable<FPointHandle>;
using FVertexRemapTable = TRemapTable<FVertexHandle>;
using FEdgeRemapTable = TRemapTable<FEdgeHandle>;
using FFaceRemapTable = TRemapTable<FFaceHandle>;

struct FRemapData
{
  FPointRemapTable Points;
  FVertexRemapTable Vertices;
  FEdgeRemapTable Edges;
  FFaceRemapTable Faces;
};

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
template<typename ElementType, typename ElementHandleType>
class THedgeElementBuffer
{
  TSparseArray<ElementType> Elements;
  uint32 Generation=1;

  friend class UHedgeKernel;

public:
  uint32 Num() const { return Elements.Num(); }
  FORCEINLINE void Reserve(uint32 const Count=0) { Elements.Reserve(Count); }
  FORCEINLINE void Reset(uint32 const Count=0)
  {
    Elements.Reset();
    Elements.Reserve(Count);
  }

  FORCEINLINE ElementHandleType Add(ElementType&& Element)
  {
    auto Index = Elements.Add(Element);
    return ElementHandleType(Index, Generation);
  }

  FORCEINLINE ElementType& Get(ElementHandleType const Handle)
  {
    auto const Index = Handle.GetIndex();
    check(Elements.IsAllocated(Index));
    return Elements[Index];
  }

  FORCEINLINE void Remove(ElementHandleType Handle)
  {
    auto const Index = Handle.GetIndex();
    check(Elements.IsAllocated(Index));
    Elements.RemoveAtUninitialized(Index);
  }

  FORCEINLINE ElementHandleType New()
  {
    auto Index = Elements.Add(ElementType());
    return ElementHandleType(Index, Generation);
  }

  template<typename... ArgsType>
  FORCEINLINE ElementHandleType New(ArgsType... Args)
  {
    return Add(ElementType(std::forward<ArgsType>(Args)...));
  }

  FORCEINLINE bool IsValidHandle(ElementHandleType const Handle) const
  {
    uint32 const HandleGeneration = Handle.GetGeneration();
    FElementIndex const Index = Handle.GetIndex();
    bool const IsValid = Elements.IsValidIndex(Index);
    if (HandleGeneration != HEDGE_IGNORED_GENERATION)
    {
      return HandleGeneration == Generation && IsValid;
    }
    return IsValid;
  }

  // Using the same approach as the MeshDescription module because that
  // is just a heck of a lot easier than the stuff I did before when
  // trying to just reuse the container and sort/swap elements around.
  void Defrag(TRemapTable<ElementHandleType>& OutRemapTable)
  {
    auto PreviousGeneration = Generation;
    ++Generation;

    OutRemapTable.Empty(Elements.GetMaxIndex());
    OutRemapTable.Add(ElementHandleType(), ElementHandleType());

    TSparseArray<ElementType> NewBuffer;
    for (typename TSparseArray<ElementType>::TIterator It( Elements ); It; ++It)
    {
      uint32 const PreviousIndex = It.GetIndex();
      uint32 const NewIndex = NewBuffer.Add(MoveTemp(*It));
      OutRemapTable.Add(
        ElementHandleType(PreviousIndex, PreviousGeneration), 
        ElementHandleType(NewIndex, Generation));
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

  THedgeElementBuffer<FHalfEdge, FEdgeHandle> Edges;
  THedgeElementBuffer<FVertex, FVertexHandle> Vertices;
  THedgeElementBuffer<FFace, FFaceHandle> Faces;
  THedgeElementBuffer<FPoint, FPointHandle> Points;

  void RemapPoints(FPointRemapTable const& Table);
  void RemapEdges(FEdgeRemapTable const& Table);
  void RemapFaces(FFaceRemapTable const& Table);
  void RemapVertices(FVertexRemapTable const& Table);

public:

  bool IsValidHandle(FEdgeHandle Handle) const;
  bool IsValidHandle(FFaceHandle Handle) const;
  bool IsValidHandle(FVertexHandle Handle) const;
  bool IsValidHandle(FPointHandle Handle) const;

  FHalfEdge& Get(FEdgeHandle Handle);
  FFace& Get(FFaceHandle Handle);
  FVertex& Get(FVertexHandle Handle);
  FPoint& Get(FPointHandle Handle);

  FHalfEdge& New(FEdgeHandle& OutHandle);
  FFace& New(FFaceHandle& OutHandle);
  FVertex& New(FVertexHandle& OutHandle);
  FPoint& New(FPointHandle& OutHandle);
  FPoint& New(FPointHandle& OutHandle, FVector Position);

  FEdgeHandle Add(FHalfEdge&& Edge);
  FFaceHandle Add(FFace&& Face);
  FVertexHandle Add(FVertex&& Vertex);
  FPointHandle Add(FPoint&& Point);

  void Remove(FEdgeHandle Handle);
  void Remove(FFaceHandle Handle);
  void Remove(FVertexHandle Handle);
  void Remove(FPointHandle Handle);

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
   * Create an empty edge and it's adjacent edge. Associate
   * the first edge with the specified face.
   * @returns The index of the first edge.
   */
  FEdgeHandle MakeEdgePair(FFaceHandle FaceHandle = FFaceHandle::Invalid);

  /**
   * Assigns all the connected edges to the specified face and assigns
   * the specified edge index to the face.
   *
   * @param FaceHandle: The face to assign and update.
   * @param RootEdgeHandle: The first edge of a loop which forms the face
   */
  void SetFace(FFaceHandle FaceHandle, FEdgeHandle RootEdgeHandle);

  /**
   * Connect the two edges specified with a new vertex associated with
   * the specified point.
   *
   * (...)[EA] -> (V<P>)[EB] -> ...
   *
   * @param EdgeHandleA: Edge which should point to EdgeIndexB as 'next'
   * @param PointHandle: Point to associate with the new vertex
   * @param EdgeHandleB: Edge originating at the new vertex pointing to EdgeIndexA as 'previous'
   * @returns The index to the newly created vertex.
   */
  FVertexHandle ConnectEdges(
    FEdgeHandle EdgeHandleA, FPointHandle PointHandle, FEdgeHandle EdgeHandleB);
};
