// Copyright 2019 Chip Collier. All Rights Reserved.

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
template <typename ElementHandleType, typename ElementType>
struct FPxElement
{
  using ProxiedType = ElementType;
  using ProxiedHandleType = ElementHandleType;

  explicit FPxElement(UHedgeKernel* Kernel, ElementHandleType Handle) noexcept
    : Kernel(Kernel)
    , Handle(Handle)
  {
  }

  explicit operator bool() const noexcept
  {
    return IsValid();
  }

  FORCEINLINE bool IsValid() const noexcept
  {
    return Kernel != nullptr && Kernel->IsValidHandle(Handle);
  }

  bool operator==(FPxElement const& Other) const
  {
    return Handle == Other.Handle && Kernel == Other.Kernel;
  }

  bool operator!=(FPxElement const& Other) const
  {
    return Handle != Other.Handle && Kernel != Other.Kernel;
  }

  FORCEINLINE ElementType& GetElement() const
  {
    return Kernel->Get(Handle);
  }

  FORCEINLINE ElementHandleType GetHandle() const
  {
    return Handle;
  }
protected:
  UHedgeKernel* Kernel;
  ElementHandleType Handle;
};

using FHalfEdgePoints = TArray<FPxPoint, TFixedAllocator<2>>;
using FHalfEdgeVertices = TArray<FPxVertex, TFixedAllocator<2>>;

/**
 * TODO: docs
 */
struct FPxHalfEdge : FPxElement<FEdgeHandle, FHalfEdge>
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
struct FPxFace : FPxElement<FFaceHandle, FFace>
{
  using FPxElement::FPxElement;

  FPxHalfEdge RootEdge() const;

  TArray<FPxHalfEdge> GetPerimeterEdges() const;
};

/**
 * TODO: docs
 */
struct FPxVertex : FPxElement<FVertexHandle, FVertex>
{
  using FPxElement::FPxElement;

  FPxHalfEdge Edge() const;
  FPxPoint Point() const;
};

/**
 * TODO: docs
 */
struct FPxPoint : FPxElement<FPointHandle, FPoint>
{
  using FPxElement::FPxElement;

  FVector Position() const;
  void SetPosition(FVector Position) const;

  FVertexSet& Vertices() const;
};
