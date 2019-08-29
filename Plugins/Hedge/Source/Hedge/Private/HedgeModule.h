// Copyright 2019 Chip Collier, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FHedgeModule : public IModuleInterface
{
public:
  /** IModuleInterface implementation */
  void StartupModule() override;
  void ShutdownModule() override;
};
