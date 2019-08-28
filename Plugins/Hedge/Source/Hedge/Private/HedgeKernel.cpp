#include "HedgeKernel.h"
#include "HedgeTypes.h"


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

FPoint* UHedgeKernel::New(FVector Pos, FPointIndex& OutIndex)
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
