// Copyright 2019 Chip Collier. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HedgeTypes.h"
#include "HedgeElements.h"
#include "HedgeKernel.generated.h"

using FPointRemapTable = TSparseArray<FPointHandle>;
using FVertexRemapTable = TSparseArray<FVertexHandle>;
using FEdgeRemapTable = TSparseArray<FEdgeHandle>;
using FFaceRemapTable = TSparseArray<FFaceHandle>;

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
  void Defrag(TSparseArray<ElementHandleType>& OutRemapTable)
  {
    ++Generation;

    OutRemapTable.Empty(Elements.GetMaxIndex());

    TSparseArray<ElementType> NewBuffer;
    for (typename TSparseArray<ElementType>::TIterator It( Elements ); It; ++It)
    {
      uint32 const PreviousIndex = It.GetIndex();
      uint32 const NewIndex = NewBuffer.Add(MoveTemp(*It));
      OutRemapTable.Insert(PreviousIndex, ElementHandleType(NewIndex, Generation));
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

  void RemapElements(FRemapData const& RemapData);

  void NewEdgePair(FEdgeHandle& OutEdge0, FEdgeHandle& OutEdge1);

public:

  HEDGE_API bool IsValidHandle(FEdgeHandle Handle) const;
  HEDGE_API bool IsValidHandle(FFaceHandle Handle) const;
  HEDGE_API bool IsValidHandle(FVertexHandle Handle) const;
  HEDGE_API bool IsValidHandle(FPointHandle Handle) const;

  HEDGE_API FHalfEdge& Get(FEdgeHandle Handle);
  HEDGE_API FFace& Get(FFaceHandle Handle);
  HEDGE_API FVertex& Get(FVertexHandle Handle);
  HEDGE_API FPoint& Get(FPointHandle Handle);

  HEDGE_API FHalfEdge& New(FEdgeHandle& OutHandle);
  HEDGE_API FFace& New(FFaceHandle& OutHandle);
  HEDGE_API FVertex& New(FVertexHandle& OutHandle);
  HEDGE_API FPoint& New(FPointHandle& OutHandle);
  HEDGE_API FPoint& New(FPointHandle& OutHandle, FVector Position);

  HEDGE_API FEdgeHandle Add(FHalfEdge&& Edge);
  HEDGE_API FFaceHandle Add(FFace&& Face);
  HEDGE_API FVertexHandle Add(FVertex&& Vertex);
  HEDGE_API FPointHandle Add(FPoint&& Point);

  HEDGE_API void Remove(FEdgeHandle Handle);
  HEDGE_API void Remove(FFaceHandle Handle);
  HEDGE_API void Remove(FVertexHandle Handle);
  HEDGE_API void Remove(FPointHandle Handle);

  HEDGE_API uint32 NumPoints() const;
  HEDGE_API uint32 NumVertices() const;
  HEDGE_API uint32 NumFaces() const;
  HEDGE_API uint32 NumEdges() const;

  template<typename ElementType>
  HEDGE_API uint32 Num() const;

  /**
   * Reorganize all element buffers into contiguous arrays
   * and updates indices on related elements.
   */
  HEDGE_API void Defrag();

  /**
   * @todo: documentssss
   */
  HEDGE_API FVertexHandle MakeVertex(
    FPointHandle PointHandle = FPointHandle::Invalid,
    FEdgeHandle EdgeHandle = FEdgeHandle::Invalid);

  /**
   * Create an empty edge and it's adjacent edge. Associate
   * the first edge with the specified face.
   * @returns The index of the first edge.
   */
  HEDGE_API FEdgeHandle MakeEdgePair(
    FPointHandle Point0Handle,
    FPointHandle Point1Handle,
    FFaceHandle FaceHandle = FFaceHandle::Invalid);

  /**
   * Create a new edge pair extending from the specified edge to the
   * specified point.
   *
   * @returns The index of the first edge.
   */
  HEDGE_API FEdgeHandle MakeEdgePair(
    FEdgeHandle PreviousEdgeHandle,
    FPointHandle PointHandle,
    FFaceHandle FaceHandle = FFaceHandle::Invalid);

  /**
   * Create a new edge pair connecting to the two specified edges.
   * This effectively "closes" the perimeter edge loop around a face.
   *
   * @returns The index of the first edge.
   */
  HEDGE_API FEdgeHandle MakeEdgePair(
    FEdgeHandle PreviousEdgeHandle,
    FEdgeHandle NextEdgeHandle,
    FFaceHandle FaceHandle = FFaceHandle::Invalid);

  /**
   * This creates a pair of half edges without any point associations.
   */
  HEDGE_API FEdgeHandle MakeEdgePair(FFaceHandle FaceHandle = FFaceHandle::Invalid);

  /**
   * Assigns all the connected edges to the specified face and assigns
   * the specified edge index to the face.
   *
   * @param FaceHandle: The face to assign and update.
   * @param RootEdgeHandle: The first edge of a loop which forms the face
   */
  HEDGE_API void SetFace(FFaceHandle FaceHandle, FEdgeHandle RootEdgeHandle);

  /**
   * Connect the two edges specified via the vertex of the second edge.
   *
   * (...)[EA] -> (V<P>)[EB] -> ...
   *
   * @param A: HalfEdge which should point to B as 'next'
   * @param B: HalfEdge which should point to A as 'previous'
   */
  HEDGE_API void ConnectEdges(FEdgeHandle A, FEdgeHandle B);

  HEDGE_API void SetVertexPoint(FVertexHandle VertexHandle, FPointHandle PointHandle);
  HEDGE_API void SetVertexEdge(FVertexHandle VertexHandle, FEdgeHandle EdgeHandle);
};


