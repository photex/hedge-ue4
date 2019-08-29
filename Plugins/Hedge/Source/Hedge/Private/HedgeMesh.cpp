// Copyright 2019 Chip Collier, Inc. All Rights Reserved.

#include "HedgeMesh.h"
#include "HedgeKernel.h"

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
