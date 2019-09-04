// Copyright 2019 Chip Collier. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HedgeTypes.generated.h"

struct FHalfEdge;
struct FFace;
struct FFaceTriangle;
struct FVertex;
struct FPoint;

struct FEdgeIndex;
struct FFaceIndex;
struct FVertexIndex;
struct FPointIndex;

using FHedgeTriangleArray = TArray<FFaceTriangle>;

using FFaceIndexSet = TSet<FFaceIndex>;
using FVertexIndexSet = TSet<FVertexIndex>;

/// Determines the upper limit of how many components can be added to a mesh.
using FOffset = uint32;
#define HEDGE_INVALID_OFFSET TNumericLimits<FOffset>::Max()
#define HEDGE_IGNORED_GENERATION 0

/**
 * Element indices encode an offset (their 'index' into a buffer)
 * and an optional generation.
 */
USTRUCT(BlueprintType)
struct FElementIndex
{
  GENERATED_BODY()

  explicit FElementIndex() noexcept
    : Offset(HEDGE_INVALID_OFFSET)
    , Generation(HEDGE_IGNORED_GENERATION)
  {
  }

  explicit FElementIndex(FOffset const Offset) noexcept
    : Offset(Offset)
    , Generation(HEDGE_IGNORED_GENERATION)
  {
  }

  explicit FElementIndex(FOffset const Offset, uint32 const Generation) noexcept
    : Offset(Offset)
    , Generation(Generation)
  {
  }

  FORCEINLINE FOffset GetOffset() const
  {
    return Offset;
  }

  FORCEINLINE uint32 GetGeneration() const
  {
    return Generation;
  }

  void Reset()
  {
    Offset = HEDGE_INVALID_OFFSET;
    Generation = HEDGE_IGNORED_GENERATION;
  }

  bool operator!=(FElementIndex const& Other) const
  {
    return !(*this == Other);
  }

  bool operator==(FElementIndex const& Other) const
  {
    bool const bOffsetMatches = Offset == Other.Offset;
    bool const bTestGeneration = Generation != HEDGE_IGNORED_GENERATION &&
      Other.Generation != HEDGE_IGNORED_GENERATION;

    if (bTestGeneration)
    {
      return bOffsetMatches && Generation == Other.Generation;
    }
    return bOffsetMatches;
  }

  friend bool operator<(FElementIndex const& Lhs, FElementIndex const& Rhs)
  {
    return Lhs.Offset < Rhs.Offset;
  }

  friend bool operator>(FElementIndex const& Lhs, FElementIndex const& Rhs)
  {
    return Lhs < Rhs;
  }

  explicit operator bool() const noexcept
  {
    return Offset < HEDGE_INVALID_OFFSET;
  }

  FString ToString() const
  {
    return (Offset == HEDGE_INVALID_OFFSET)
             ? TEXT("Invalid")
             : FString::Printf(TEXT("%d"), Offset);
  }

  friend FArchive& operator<<(FArchive& Ar, FElementIndex& Element)
  {
    // TODO: We need to serialize the generation, probably fixed at '1'
    Ar << Element.Offset;
    return Ar;
  }

protected:
  FOffset Offset;
  uint32 Generation;
};


USTRUCT(BlueprintType)
struct FEdgeIndex : public FElementIndex
{
  GENERATED_BODY()
  using FElementIndex::FElementIndex;
  FORCEINLINE friend uint32 GetTypeHash(FEdgeIndex const& Other)
  {
    return GetTypeHash(Other.Offset);
  }

  HEDGE_API static const FEdgeIndex Invalid;
};


USTRUCT(BlueprintType)
struct FFaceIndex : public FElementIndex
{
  GENERATED_BODY()
  using FElementIndex::FElementIndex;
  FORCEINLINE friend uint32 GetTypeHash(FFaceIndex const& Other)
  {
    return GetTypeHash(Other.Offset);
  }

  HEDGE_API static const FFaceIndex Invalid;
};


USTRUCT(BlueprintType)
struct FVertexIndex : public FElementIndex
{
  GENERATED_BODY()
  using FElementIndex::FElementIndex;
  FORCEINLINE friend uint32 GetTypeHash(FVertexIndex const& Other)
  {
    return GetTypeHash(Other.Offset);
  }

  HEDGE_API static const FVertexIndex Invalid;
};


USTRUCT(BlueprintType)
struct FPointIndex : public FElementIndex
{
  GENERATED_BODY()
  using FElementIndex::FElementIndex;
  FORCEINLINE friend uint32 GetTypeHash(FPointIndex const& Other)
  {
    return GetTypeHash(Other.Offset);
  }

  HEDGE_API static const FPointIndex Invalid;
};
