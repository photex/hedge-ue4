// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "hedge.h"

#define LOCTEXT_NAMESPACE "FHedgeModule"

FEdgeIndex const FEdgeIndex::Invalid(INVALID_OFFSET);
FFaceIndex const FFaceIndex::Invalid(INVALID_OFFSET);
FVertexIndex const FVertexIndex::Invalid(INVALID_OFFSET);
FPointIndex const FPointIndex::Invalid(INVALID_OFFSET);

///////////////////////////////////////////////////////////////////////////////

FHalfEdge* UHedgeKernel::Get(FEdgeIndex Index)
{
  return nullptr;
}

FFace* UHedgeKernel::Get(FFaceIndex Index)
{
  return nullptr;
}

FVertex* UHedgeKernel::Get(FVertexIndex Index)
{
  return nullptr;
}

FPoint* UHedgeKernel::Get(FPointIndex Index)
{
  return nullptr;
}

FHalfEdge* UHedgeKernel::New(FEdgeIndex& OutIndex)
{
  return nullptr;
}

FFace* UHedgeKernel::New(FFaceIndex& OutIndex)
{
  return nullptr;
}

FVertex* UHedgeKernel::New(FVertexIndex& OutIndex)
{
  return nullptr;
}

FPoint* UHedgeKernel::New(FPosition Pos, FPointIndex& OutIndex)
{
  return nullptr;
}

FEdgeIndex UHedgeKernel::Insert(FHalfEdge Edge)
{
  return FEdgeIndex::Invalid;
}

FFaceIndex UHedgeKernel::Insert(FFace Face)
{
  return FFaceIndex::Invalid;
}

FVertexIndex UHedgeKernel::Insert(FVertex Vertex)
{
  return FVertexIndex::Invalid;
}

FPointIndex UHedgeKernel::Insert(FPoint Point)
{
  return FPointIndex::Invalid;
}

void UHedgeKernel::Remove(FEdgeIndex Index)
{
}

void UHedgeKernel::Remove(FFaceIndex Index)
{
}

void UHedgeKernel::Remove(FVertexIndex Index)
{
}

void UHedgeKernel::Remove(FPointIndex Index)
{
}

uint32 UHedgeKernel::PointCount() const
{
  return 0;
}

uint32 UHedgeKernel::VertexCount() const
{
  return 0;
}

uint32 UHedgeKernel::FaceCount() const
{
  return 0;
}

uint32 UHedgeKernel::EdgeCount() const
{
  return 0;
}

void UHedgeKernel::Defrag()
{
}

FEdgeIndex UHedgeKernel::MakeEdgePair()
{
  return FEdgeIndex::Invalid;
}

FFaceIndex UHedgeKernel::MakeFace(FEdgeIndex RootEdgeIndex)
{
  return FFaceIndex::Invalid;
}

FVertexIndex UHedgeKernel::ConnectEdges(
  FEdgeIndex EdgeIndexA, FPointIndex PointIndex, FEdgeIndex EdgeIndexB)
{
  return FVertexIndex::Invalid;
}

///////////////////////////////////////////////////////////////////////////////

void FHedgeModule::StartupModule()
{
  // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}


void FHedgeModule::ShutdownModule()
{
  // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
  // we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHedgeModule, Hedge)
