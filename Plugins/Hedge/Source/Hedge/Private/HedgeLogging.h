#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHedge, Log, All);

#define DebugLog(Msg) UE_LOG(LogHedge, Verbose, TEXT(Msg))
#define DebugLogV(Msg, ...) UE_LOG(LogHedge, Verbose, TEXT(Msg), __VA_ARGS__)

#define InfoLog(Msg) UE_LOG(LogHedge, Log, TEXT(Msg))
#define InfoLogV(Msg, ...) UE_LOG(LogHedge, Log, TEXT(Msg), __VA_ARGS__)

#define WarningLog(Msg) UE_LOG(LogHedge, Warning, TEXT(Msg))
#define WarningLogV(Msg, ...) UE_LOG(LogHedge, Warning, TEXT(Msg), __VA_ARGS__)

#define ErrorLog(Msg) UE_LOG(LogHedge, Error, TEXT(Msg))
#define ErrorLogV(Msg, ...) UE_LOG(LogHedge, Error, TEXT(Msg), __VA_ARGS__)
