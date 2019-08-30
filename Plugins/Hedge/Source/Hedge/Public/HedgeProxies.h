// Copyright 2019 Chip Collier, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HedgeTypes.h"

class UHedgeKernel;

struct FPxHalfEdge;
struct FPxFace;
struct FPxVertex;
struct FPxPoint;

/**
 * TODO: docs
 */
template <typename ElementIndexType, typename ElementType>
struct FPxElement
{
  explicit FPxElement(UHedgeKernel* Kernel, ElementIndexType Index) noexcept
    : Kernel(Kernel)
    , Index(Index)
  {
  }

  explicit operator bool() const noexcept
  {
    return Kernel != nullptr && static_cast<bool>(Index);
  }

  bool operator==(FPxElement const& Other) const
  {
    return Index == Other.Index && Kernel == Other.Kernel;
  }

  bool operator!=(FPxElement const& Other) const
  {
    return Index != Other.Index && Kernel != Other.Kernel;
  }

  ElementType& GetElement() const
  {
    return Kernel->Get(Index);
  }

  ElementIndexType GetIndex()
  {
    return Index;
  }
protected:
  UHedgeKernel* Kernel;
  ElementIndexType Index;
};

using FHalfEdgePoints = TArray<FPxPoint, TFixedAllocator<2>>;
using FHalfEdgeVertices = TArray<FPxVertex, TFixedAllocator<2>>;

/**
 * TODO: docs
 */
struct FPxHalfEdge : FPxElement<FEdgeIndex, FHalfEdge>
{
  using FPxElement::FPxElement;

  FPxVertex Vertex() const;
  FPxFace Face() const;
  FPxHalfEdge Next() const;
  FPxHalfEdge Prev() const;
  FPxHalfEdge Adjacent() const;

  bool IsBoundary() const;

  FHalfEdgePoints GetPoints() const;
  FHalfEdgeVertices GetVertices() const;
};

/**
 * TODO: docs
 */
struct FPxFace : FPxElement<FFaceIndex, FFace>
{
  using FPxElement::FPxElement;

  FPxHalfEdge RootEdge() const;
};

/**
 * TODO: docs
 */
struct FPxVertex : FPxElement<FVertexIndex, FVertex>
{
  using FPxElement::FPxElement;

  FPxHalfEdge Edge() const;
  FPxPoint Point() const;
};

/**
 * TODO: docs
 */
struct FPxPoint : FPxElement<FPointIndex, FPoint>
{
  using FPxElement::FPxElement;

  FVector Position() const;
  void SetPosition(FVector Position) const;

  FVertexIndexSet& Vertices() const;
};
