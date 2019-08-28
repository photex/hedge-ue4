// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Hedge.generated.h"


struct FHalfEdge;
struct FFace;
struct FFaceTriangle;
struct FVertex;
struct FPoint;

/// Strongly typed index for HalfEdges.
struct FEdgeIndex;
/// Strongly typed index for Faces.
struct FFaceIndex;
/// Strongly typed index for Vertices.
struct FVertexIndex;
/// Strongly typed index for Points.
struct FPointIndex;

class UPxHalfEdge;
class UPxFace;
class UPxVertex;
class UPxPoint;

class UHedgeKernel;
class UHedgeMesh;
class UHedgeMeshBuilder;
class UHedgeElementLoopBuilder;

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

//////////////////////////////////////////////////////////////
/// Principle structures to encode/define a mesh

UENUM()
enum class EMeshElementStatus : uint16
{
  /// This element is actively contributing to the topology of a mesh.
  Active = 0x0000,
  /// This element is available for re-use or removal.
  Inactive = 0x8000,
};

/**
 * Common fields used in every mesh element
 */
USTRUCT(BlueprintType)
struct FMeshElement
{
  GENERATED_BODY()

  /// Elements that are inactive no longer contribute to the mesh
  /// and the cells they occupy can be re-used or collected.
  EMeshElementStatus Status = EMeshElementStatus::Active;
  /// The Tag field can be used generically as a discriminator if
  /// a modifying process needs it.
  uint16 Tag = 0;
  /// Using 1 as the default so that 0 can have another meaning (ignored).
  FGeneration Generation = 1;
};

/**
 * Principle structure to encode the connectivity of a mesh.
 */
USTRUCT(BlueprintType)
struct FHalfEdge : public FMeshElement
{
  GENERATED_BODY()
  /// The vertex this edge starts from
  FVertexIndex VertexIndex;
  /// Either the face that this edge contributes to
  /// or 'Invalid' for boundary edges.
  FFaceIndex FaceIndex;
  /// The next edge in the loop that forms a face.
  FEdgeIndex NextEdgeIndex;
  /// The previous edge in the loop that forms a face.
  FEdgeIndex PrevEdgeIndex;
  /// The adjacent 'twin' half edge.
  FEdgeIndex AdjacentEdgeIndex;
};

/**
 * Faces are formed by a directed loop of edges and represent
 * a renderable element of a mesh.
 * Faces with greater than 3 vertices in their boundary maintain
 * the list of triangles that they must be comprised of to be rendered.
 */
USTRUCT(BlueprintType)
struct FFace : public FMeshElement
{
  GENERATED_BODY()
  /// The first edge of a loop that forms the face.
  FEdgeIndex RootEdgeIndex;
  /// A list of the triangles that compose this face.
  /// (Perhaps empty when the face itself is already a triangle)
  FHedgeFaceTriangleArray Triangles;
};

/**
 * Encodes the 3 vertices (in counter clockwise order) of a
 * sub-triangle of a given face.
 */
USTRUCT(BlueprintType)
struct FFaceTriangle
{
  GENERATED_BODY()
  FVertexIndex VertexIndex0;
  FVertexIndex VertexIndex1;
  FVertexIndex VertexIndex2;
};

/**
 * Vertices represent the connection of two edges.
 * Each vertex has an associated point which holds
 * attributes shared by all associated vertices.
 */
USTRUCT(BlueprintType)
struct FVertex : public FMeshElement
{
  GENERATED_BODY()
  /// The point which holds any relevant attributes.
  FPointIndex PointIndex;
  /// The edge eminating from this vertex.
  FEdgeIndex EdgeIndex;
};

/**
 * Points are the structure which holds the common
 * vertex attribute 'position'.
 * Multiple vertices may be associated with a point.
 * @todo There needs to be a general attribute system in place.
 */
USTRUCT(BlueprintType)
struct FPoint : public FMeshElement
{
  GENERATED_BODY()
  /// The location of this point.
  FPosition Position;
  /// The associated vertices
  FHedgeVertexArray Vertices;
};

//////////////////////////////////////////////////////////////
/// The mesh kernel contains element buffers and provides
/// the fundamental utilities. It's meant to be low level and
/// is probably not fun to use.

UCLASS()
class UHedgeKernel final : public UObject
{
  GENERATED_BODY()

  FHalfEdge* Get(FEdgeIndex Index);
  FFace* Get(FFaceIndex Index);
  FVertex* Get(FVertexIndex Index);
  FPoint* Get(FPointIndex Index);

  FHalfEdge* New(FEdgeIndex& OutIndex);
  FFace* New(FFaceIndex& OutIndex);
  FVertex* New(FVertexIndex& OutIndex);
  FPoint* New(FPosition Pos, FPointIndex& OutIndex);

  FEdgeIndex Insert(FHalfEdge Edge);
  FFaceIndex Insert(FFace Face);
  FVertexIndex Insert(FVertex Vertex);
  FPointIndex Insert(FPoint Point);

  void Remove(FEdgeIndex Index);
  void Remove(FFaceIndex Index);
  void Remove(FVertexIndex Index);
  void Remove(FPointIndex Index);

  uint32 PointCount() const;
  uint32 VertexCount() const;
  uint32 FaceCount() const;
  uint32 EdgeCount() const;

  /**
   * Sorts all buffers, moving any inactive elements to the back,
   * and updates all connectivity information.
   */
  void Defrag();

  /**
   * Create an empty edge and it's adjacent edge.
   * @returns The index of the first edge.
   */
  FEdgeIndex MakeEdgePair();

  /**
   * Create a new face, given a valid edge loop. If the face has more
   * than 3 sides it will also build it's triangle list.
   *
   * @param RootEdgeIndex: The first edge of a loop which forms the face
   * @returns The index of the newly created face.
   */
  FFaceIndex MakeFace(FEdgeIndex RootEdgeIndex);

  /**
   * Connect the two edges specified with a new vertex associated with
   * the specified point.
   * @param EdgeIndexA: Edge which points toward the new vertex
   * @param PointIndex: Point to associate with the new vertex
   * @param EdgeIndexB: Edge which points out from the new vertex
   */
  FVertexIndex ConnectEdges(
    FEdgeIndex EdgeIndexA, FPointIndex PointIndex, FEdgeIndex EdgeIndexB);
};

//////////////////////////////////////////////////////////////
/// Mesh element proxies are meant to provide easy topology
/// traversal and common calculations that could be useful
/// to higher level systems or operators.


//////////////////////////////////////////////////////////////
/// Module interface.

class FHedgeModule : public IModuleInterface
{
public:
  /** IModuleInterface implementation */
  void StartupModule() override;
  void ShutdownModule() override;
};
