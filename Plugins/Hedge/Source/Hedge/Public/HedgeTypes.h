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

/**
 * Element indices encode an offset (their 'index' into a buffer)
 * and an optional generation.
 */
USTRUCT(BlueprintType)

struct FElementIndex
{
  GENERATED_BODY()

  explicit FElementIndex() noexcept
    : OffsetVal(HEDGE_INVALID_OFFSET)
  {
  }

  explicit FElementIndex(FOffset const Offset) noexcept
    : OffsetVal(Offset)
  {
  }

  FORCEINLINE FOffset Offset() const
  {
    return OffsetVal;
  }

  void Reset()
  {
    OffsetVal = HEDGE_INVALID_OFFSET;
  }

  bool operator!=(FElementIndex const& Other) const
  {
    return !(*this == Other);
  }

  bool operator==(FElementIndex const& Other) const
  {
    return OffsetVal == Other.OffsetVal;
  }

  friend bool operator<(FElementIndex const& Lhs, FElementIndex const& Rhs)
  {
    return Lhs.OffsetVal < Rhs.OffsetVal;
  }

  friend bool operator>(FElementIndex const& Lhs, FElementIndex const& Rhs)
  {
    return Lhs < Rhs;
  }

  explicit operator bool() const noexcept
  {
    return OffsetVal < HEDGE_INVALID_OFFSET;
  }

  FString ToString() const
  {
    return (OffsetVal == HEDGE_INVALID_OFFSET)
             ? TEXT("Invalid")
             : FString::Printf(TEXT("%d"), OffsetVal);
  }

  friend FArchive& operator<<(FArchive& Ar, FElementIndex& Element)
  {
    Ar << Element.OffsetVal;
    return Ar;
  }

protected:
  FOffset OffsetVal;
};


USTRUCT(BlueprintType)

struct FEdgeIndex : public FElementIndex
{
  GENERATED_BODY()
  using FElementIndex::FElementIndex;
  FORCEINLINE friend uint32 GetTypeHash(FEdgeIndex const& Other)
  {
    return GetTypeHash(Other.OffsetVal);
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
    return GetTypeHash(Other.OffsetVal);
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
    return GetTypeHash(Other.OffsetVal);
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
    return GetTypeHash(Other.OffsetVal);
  }

  HEDGE_API static const FPointIndex Invalid;
};
