// Copyright 2019 Chip Collier, Inc. All Rights Reserved.

#include "HedgeMesh.h"
#include "HedgeKernel.h"
#include "HedgeProxies.h"

UHedgeMesh::UHedgeMesh()
  : Kernel(nullptr)
{
  Kernel = CreateDefaultSubobject<UHedgeKernel>(TEXT("MeshKernel"));
}

void UHedgeMesh::GetStats(FHedgeMeshStats& OutStats) const
{
  OutStats.NumPoints = Kernel->NumPoints();
  OutStats.NumVertices = Kernel->NumVertices();
  OutStats.NumEdges = Kernel->NumEdges();
  OutStats.NumFaces = Kernel->NumFaces();
}

FPxFace UHedgeMesh::Face(FFaceIndex const& Index) const
{
  return FPxFace(Kernel, Index);
}

FPxHalfEdge UHedgeMesh::Edge(FEdgeIndex const& Index) const
{
  return FPxHalfEdge(Kernel, Index);
}

FPxPoint UHedgeMesh::Point(FPointIndex const& Index) const
{
  return FPxPoint(Kernel, Index);
}

FPxVertex UHedgeMesh::Vertex(FVertexIndex const& Index) const
{
  return FPxVertex(Kernel, Index);
}

void UHedgeMesh::Dissolve(FEdgeIndex Index)
{
}

void UHedgeMesh::Dissolve(FFaceIndex Index)
{
}

void UHedgeMesh::Dissolve(FVertexIndex Index)
{
}

void UHedgeMesh::Dissolve(FPointIndex Index)
{
}
