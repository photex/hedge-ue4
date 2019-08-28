// Copyright 2019 Chip Collier, Inc. All Rights Reserved.

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

// Common container aliases
using FHedgeFaceArray = TArray<FFace>;
using FHedgeFaceTriangleArray = TArray<FFaceTriangle>;
using FHedgeHalfEdgeArray = TArray<FHalfEdge>;
using FHedgeVertexArray = TArray<FVertex>;
using FHedgePointArray = TArray<FPoint>;

using FEdgeIndexSet = TSet<FEdgeIndex>;
using FFaceIndexSet = TSet<FFaceIndex>;
using FVertexIndexSet = TSet<FVertexIndex>;

// Common attribute aliases
using FPosition = FVector;
using FNormal = FVector;
using FTexCoord = FVector2D;

/// Determines the upper limit of how many components can be added to a mesh.
using FOffset = uint32;
#define INVALID_OFFSET 0

/// Each cell tracks how many times it's be (re)used.
using FGeneration = uint32; // TODO: kinda large isn't it?
#define IGNORED_GENERATION 0

/**
 * Element indices encode an offset (their 'index' into a buffer)
 * and an optional generation.
 * The generation is used to validate indices held by other systems.
 * A generation value of '0' allows another system to request an
 * element regardless of any concern of whether it has been
 * modified or replaced.
 */
USTRUCT(BlueprintType)

struct FElementIndex
{
  GENERATED_BODY()

  explicit FElementIndex() noexcept
    : OffsetVal(INVALID_OFFSET)
    , GenerationVal(IGNORED_GENERATION)
  {
  }

  explicit FElementIndex(FOffset const Offset) noexcept
    : OffsetVal(Offset)
    , GenerationVal(IGNORED_GENERATION)
  {
  }

  explicit FElementIndex(FOffset const Offset, FGeneration const Generation) noexcept
    : OffsetVal(Offset)
    , GenerationVal(Generation)
  {
  }

  FORCEINLINE FOffset Offset() const
  {
    return OffsetVal;
  }

  FORCEINLINE FGeneration Generation() const
  {
    return GenerationVal;
  }

  void Reset()
  {
    OffsetVal = INVALID_OFFSET;
    GenerationVal = IGNORED_GENERATION;
  }

  bool operator!=(FElementIndex const& Other) const
  {
    return !(*this == Other);
  }

  bool operator==(FElementIndex const& Other) const
  {
    return OffsetVal == Other.OffsetVal &&
      GenerationVal == Other.GenerationVal;
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
    return OffsetVal > INVALID_OFFSET;
  }

  FString ToString() const
  {
    return (OffsetVal == INVALID_OFFSET)
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
  FGeneration GenerationVal;
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
