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
struct FFaceHandle;
struct FVertexHandle;
struct FPointHandle;

using FHedgeTriangleArray = TArray<FFaceTriangle>;

using FFaceIndexSet = TSet<FFaceHandle>;
using FVertexIndexSet = TSet<FVertexHandle>;

/// Determines the upper limit of how many components can be added to a mesh.
using FOffset = uint32;
#define HEDGE_INVALID_OFFSET TNumericLimits<FOffset>::Max()
#define HEDGE_IGNORED_GENERATION 0

/**
 * Element indices encode an offset (their 'index' into a buffer)
 * and an optional generation.
 */
USTRUCT(BlueprintType)
struct FElementHandle
{
  GENERATED_BODY()

  explicit FElementHandle() noexcept
    : Offset(HEDGE_INVALID_OFFSET)
    , Generation(HEDGE_IGNORED_GENERATION)
  {
  }

  explicit FElementHandle(FOffset const Offset) noexcept
    : Offset(Offset)
    , Generation(HEDGE_IGNORED_GENERATION)
  {
  }

  explicit FElementHandle(FOffset const Offset, uint32 const Generation) noexcept
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

  bool operator!=(FElementHandle const& Other) const
  {
    return !(*this == Other);
  }

  bool operator==(FElementHandle const& Other) const
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

  friend bool operator<(FElementHandle const& Lhs, FElementHandle const& Rhs)
  {
    return Lhs.Offset < Rhs.Offset;
  }

  friend bool operator>(FElementHandle const& Lhs, FElementHandle const& Rhs)
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

  friend FArchive& operator<<(FArchive& Ar, FElementHandle& Element)
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
struct FEdgeIndex : public FElementHandle
{
  GENERATED_BODY()
  using FElementHandle::FElementHandle;
  FORCEINLINE friend uint32 GetTypeHash(FEdgeIndex const& Other)
  {
    return GetTypeHash(Other.Offset);
  }

  HEDGE_API static const FEdgeIndex Invalid;
};


USTRUCT(BlueprintType)
struct FFaceHandle : public FElementHandle
{
  GENERATED_BODY()
  using FElementHandle::FElementHandle;
  FORCEINLINE friend uint32 GetTypeHash(FFaceHandle const& Other)
  {
    return GetTypeHash(Other.Offset);
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
    return GetTypeHash(Other.Offset);
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
    return GetTypeHash(Other.Offset);
  }

  HEDGE_API static const FPointHandle Invalid;
};
