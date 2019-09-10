// Copyright 2019 Chip Collier. All Rights Reserved.

#include "CoreTypes.h"
#include "Misc/AutomationTest.h"
#include "HedgeTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// Smoke test to validate our desired behavior for element handles
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FHedgeElementHandleTests, "Hedge.ElementHandles",
  EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter
)

bool FHedgeElementHandleTests::RunTest(const FString& Parameters)
{
  FElementHandle DefaultHandle;

  TestFalse(TEXT("Default constructed handle is invalid"), 
    static_cast<bool>(DefaultHandle));

  DefaultHandle = FElementHandle(1);
  TestTrue(TEXT("Element handle with a non-zero index is valid"),
    static_cast<bool>(DefaultHandle));

  DefaultHandle.Reset();
  TestFalse(TEXT("Resetting a handle makes it invalid."), 
    static_cast<bool>(DefaultHandle));

  FElementHandle HandleA, HandleB;
  TestEqual(TEXT("Two default constructed handles are equal."), HandleA, HandleB);

  HandleA = FElementHandle(1);
  TestNotEqual(TEXT("Handles with different index values are not equal."),
    HandleA, HandleB);

  HandleB = FElementHandle(1);
  TestEqual(
    TEXT("Handles with matching index values are equal."),
    HandleA, HandleB);

  HandleB = FElementHandle(1, 2);
  TestNotEqual(
    TEXT("Handles with matching index values but different generations are not equal."),
    HandleA, HandleB);

  HandleA = FElementHandle(1, 2);
  TestEqual(
    TEXT("Handles with matching index values and generations are equal."),
    HandleA, HandleB);

  return true;
}


#endif