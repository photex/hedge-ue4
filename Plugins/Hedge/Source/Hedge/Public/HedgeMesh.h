// Copyright 2019 Chip Collier. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HedgeTypes.h"
#include "HedgeKernel.h"
#include "HedgeMesh.generated.h"

struct FPxHalfEdge;
struct FPxFace;
struct FPxVertex;
struct FPxPoint;

class UHedgeMesh;


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
 * @todo: docs
 */
template<typename ProxyType>
class THedgeElementIterator
{
  using FHandle = typename ProxyType::ProxiedHandleType;
  explicit THedgeElementIterator(UHedgeKernel* Kernel, FHandle Handle)
    : CurrentHandle(Handle)
    , Kernel(Kernel)
  {
    if (Kernel && !Kernel->IsValidHandle(CurrentHandle))
    {
      FindNextValidHandle();
    }
  }

  THedgeElementIterator()
    : CurrentHandle()
    , Kernel(nullptr)
  {}

public:
  friend class UHedgeMesh;
  template<typename>
  friend struct THedgeElementRangeAdaptor;

  THedgeElementIterator operator++()
  {
    if (CurrentHandle)
    {
      FindNextValidHandle();
    }
    return *this;
  }

  ProxyType operator*() const
  {
    return ProxyType(Kernel, CurrentHandle);
  }

  bool operator!=(THedgeElementIterator const& Other) const
  {
    return CurrentHandle != Other.CurrentHandle;
  }

private:
  void FindNextValidHandle()
  {
    auto const ElementCount = Kernel->Num<typename ProxyType::ProxiedType>();
    ++CurrentHandle.Index;
    while(CurrentHandle.Index < ElementCount 
      && !Kernel->IsValidHandle(CurrentHandle))
    {
      ++CurrentHandle.Index;
    }
    if (CurrentHandle.Index >= ElementCount)
    {
      CurrentHandle = FHandle();
    }
  }
  FHandle CurrentHandle;
  UHedgeKernel* Kernel;
};

template<typename ElementProxyType>
struct THedgeElementRangeAdaptor
{
  using FHandle = typename ElementProxyType::ProxiedHandleType;
  using FIterator = THedgeElementIterator<ElementProxyType>;

  FIterator begin()
  {
    FHandle InitialHandle;
    if (Kernel->Num<typename ElementProxyType::ProxiedType>() > 0)
    {
      InitialHandle = FHandle(0);
    }
    return FIterator(Kernel, InitialHandle);
  }

  FIterator end()
  {
    return FIterator();
  }

  explicit THedgeElementRangeAdaptor(UHedgeKernel* Kernel)
    : Kernel(Kernel)
  {}

private:
  UHedgeKernel* Kernel;
};

/**
 * This is the "high level" interface for building and modifying hedge
 * meshes.
 *
 * Little by little this interface should grow to provide the most common
 * facilities needed for a modeling tool
 */
UCLASS()
class UHedgeMesh final : public UObject
{
  GENERATED_BODY()

  UPROPERTY()
  UHedgeKernel* Kernel;

public:
  using FFaceRangeIterator = THedgeElementRangeAdaptor<FPxFace>;
  using FHalfEdgeRangeIterator = THedgeElementRangeAdaptor<FPxHalfEdge>;
  using FVertexRangeIterator = THedgeElementRangeAdaptor<FPxVertex>;
  using FPointRangeIterator = THedgeElementRangeAdaptor<FPxPoint>;

  UHedgeMesh();

  void GetStats(FHedgeMeshStats& OutStats) const;

  /// Perhaps just an escape-hatch for an incomplete mesh API? :shrug:
  UHedgeKernel* GetKernel() const;

  FPxFace Face(uint32 Index) const;
  FPxFace Face(FFaceHandle const& Handle) const;
  FFaceRangeIterator Faces() const;

  FPxHalfEdge Edge(uint32 Index) const;
  FPxHalfEdge Edge(FEdgeHandle const& Handle) const;
  FHalfEdgeRangeIterator Edges() const;

  FPxPoint Point(uint32 Index) const;
  FPxPoint Point(FPointHandle const& Handle) const;
  FPointRangeIterator Points() const;

  FPxVertex Vertex(uint32 Index) const;
  FPxVertex Vertex(FVertexHandle const& Handle) const;
  FVertexRangeIterator Vertices() const;

  /**
   * Given an array of positions, new points are added to the mesh.
   */
  TArray<FPointHandle> AddPoints(TArray<FVector> const& Positions) const;
  /**
   * Given an array of positions, new points are added to the mesh.
   *
   * @note: This is the variant of this method that actually adds points to the mesh.
   *        In cases where you aren't building an array using TArray this is perhaps
   *        the more convenient option.
   */
  TArray<FPointHandle> AddPoints(FVector const Positions[], uint32 PositionCount) const;

  /**
   * Given an array of points; create all required mesh elements to form a face.
   *
   * @note: It is assumed that the points are specified in the correct winding order.
   */
  FFaceHandle AddFace(TArray<FPointHandle> const& Points);

  FFaceHandle AddFace(FPointHandle const Points[], uint32 PointCount);

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
  FFaceHandle AddFace(FEdgeHandle const& RootEdgeHandle, TArray<FPointHandle> const& Points);

  /**
   * Given an edge and an array of points; Create all required mesh elements to
   * create a new face (triangle) extending from the specified edge.
   *
   * @note: It is assumed that the specified edge is a suitable boundary edge to form the face.
   */
  FFaceHandle AddFace(FEdgeHandle const& RootEdgeHandle, FPointHandle const& PointHandle);

  FFaceHandle AddFace(FEdgeHandle const& RootEdgeHandle, FVector Position);

  FFaceHandle AddFace(TArray<FEdgeHandle> const& Edges, TArray<FPointHandle> const& Points);

  /**
   * Given a list of edges; Connect each edge and create a new face.
   *
   * @note: It is assumed that the specified edges are "related" in such a way that they
   *        can be connected to form a face.
   */
  FFaceHandle AddFace(TArray<FEdgeHandle> const& Edges);

  FFaceHandle AddFace(FEdgeHandle const Edges[], uint32 EdgeCount);

  /**
   * @todo docs
   */
  FFaceHandle AddFace(FEdgeHandle const& RootEdge);

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
  void Dissolve(FEdgeHandle Handle);

  /**
   * Removes the specified face and updates the associated edges.
   */
  void Dissolve(FFaceHandle Handle);

  /**
   * Oh my
   */
  void Dissolve(FVertexHandle Handle);

  /**
   * Nuke it from orbit why dontcha
   */
  void Dissolve(FPointHandle Handle);
};
