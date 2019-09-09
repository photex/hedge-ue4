// Copyright 2019 Chip Collier. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HedgeTypes.generated.h"

struct FHalfEdge;
struct FFace;
struct FFaceTriangle;
struct FVertex;
struct FPoint;

struct FEdgeHandle;
struct FFaceHandle;
struct FVertexHandle;
struct FPointHandle;

using FHedgeTriangleArray = TArray<FFaceTriangle>;

using FFaceSet = TSet<FFaceHandle>;
using FVertexSet = TSet<FVertexHandle>;

/// Determines the upper limit of how many components can be added to a mesh.
using FElementIndex = uint32;
using FElementGeneration = uint32;
#define HEDGE_INVALID_INDEX TNumericLimits<FElementIndex>::Max()
#define HEDGE_IGNORED_GENERATION 0

/**
 * Element handles encode an index and an optional generation.
 *
 * The generation field is used to invalidate handles returned by the
 * kernel after a defrag ("compact") operation was performed as many
 * element indices will have been invalidated.
 */
USTRUCT(BlueprintType)
struct FElementHandle
{
  GENERATED_BODY()

  explicit FElementHandle() noexcept
    : Index(HEDGE_INVALID_INDEX)
    , Generation(HEDGE_IGNORED_GENERATION)
  {
  }

  explicit FElementHandle(FElementIndex const Index) noexcept
    : Index(Index)
    , Generation(HEDGE_IGNORED_GENERATION)
  {
  }

  explicit FElementHandle(FElementIndex const Index, uint32 const Generation) noexcept
    : Index(Index)
    , Generation(Generation)
  {
  }

  FORCEINLINE FElementIndex GetIndex() const
  {
    return Index;
  }

  FORCEINLINE FElementGeneration GetGeneration() const
  {
    return Generation;
  }

  void Reset()
  {
    Index = HEDGE_INVALID_INDEX;
    Generation = HEDGE_IGNORED_GENERATION;
  }

  bool operator!=(FElementHandle const& Other) const
  {
    return !(*this == Other);
  }

  bool operator==(FElementHandle const& Other) const
  {
    bool const bIndexMatches = Index == Other.Index;
    bool const bTestGeneration = Generation != HEDGE_IGNORED_GENERATION ||
      Other.Generation != HEDGE_IGNORED_GENERATION;

    if (bTestGeneration)
    {
      return bIndexMatches && Generation == Other.Generation;
    }
    return bIndexMatches;
  }

  friend bool operator<(FElementHandle const& Lhs, FElementHandle const& Rhs)
  {
    return Lhs.Index < Rhs.Index;
  }

  friend bool operator>(FElementHandle const& Lhs, FElementHandle const& Rhs)
  {
    return Lhs < Rhs;
  }

  explicit operator bool() const noexcept
  {
    return Index < HEDGE_INVALID_INDEX;
  }

  FString ToString() const
  {
    return (Index == HEDGE_INVALID_INDEX)
             ? TEXT("Invalid")
             : FString::Printf(TEXT("%d"), Index);
  }

  friend FArchive& operator<<(FArchive& Ar, FElementHandle& Element)
  {
    // TODO: We need to serialize the generation, probably fixed at '1'
    Ar << Element.Index;
    return Ar;
  }

protected:
  FElementIndex Index;
  FElementGeneration Generation;
};


USTRUCT(BlueprintType)
struct FEdgeHandle : public FElementHandle
{
  GENERATED_BODY()
  using FElementHandle::FElementHandle;
  FORCEINLINE friend uint32 GetTypeHash(FEdgeHandle const& Other)
  {
    return GetTypeHash(Other.Index);
  }

  HEDGE_API static const FEdgeHandle Invalid;
};


USTRUCT(BlueprintType)
struct FFaceHandle : public FElementHandle
{
  GENERATED_BODY()
  using FElementHandle::FElementHandle;
  FORCEINLINE friend uint32 GetTypeHash(FFaceHandle const& Other)
  {
    return GetTypeHash(Other.Index);
  }

  HEDGE_API static const FFaceHandle Invalid;
};


USTRUCT(BlueprintType)
struct FVertexHandle : public FElementHandle
{
  GENERATED_BODY()
  using FElementHandle::FElementHandle;
  FORCEINLINE friend uint32 GetTypeHash(FVertexHandle const& Other)
  {
    return GetTypeHash(Other.Index);
  }

  HEDGE_API static const FVertexHandle Invalid;
};


USTRUCT(BlueprintType)
struct FPointHandle : public FElementHandle
{
  GENERATED_BODY()
  using FElementHandle::FElementHandle;
  FORCEINLINE friend uint32 GetTypeHash(FPointHandle const& Other)
  {
    return GetTypeHash(Other.Index);
  }

  HEDGE_API static const FPointHandle Invalid;
};
