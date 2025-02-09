// Copyright 2020 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "UsdWorld.h"
#include "UsdBridge/UsdBridge.h"
#include "UsdInstance.h"
#include "UsdDevice.h"
#include "UsdDataArray.h"

DEFINE_PARAMETER_MAP(UsdWorld,
  REGISTER_PARAMETER_MACRO("name", ANARI_STRING, name)
  REGISTER_PARAMETER_MACRO("usd::name", ANARI_STRING, usdName)
  REGISTER_PARAMETER_MACRO("usd::timevarying", ANARI_INT32, timeVarying)
  REGISTER_PARAMETER_MACRO("instance", ANARI_ARRAY, instances)
)


UsdWorld::UsdWorld(const char* name, UsdBridge* bridge)
  : BridgedBaseObjectType(ANARI_WORLD, name, bridge)
{
}

UsdWorld::~UsdWorld()
{
#ifdef OBJECT_LIFETIME_EQUALS_USD_LIFETIME
  if(usdBridge)
    usdBridge->DeleteWorld(usdHandle);
#endif
}

void UsdWorld::filterSetParam(const char *name,
  ANARIDataType type,
  const void *mem,
  UsdDevice* device)
{
  if (filterNameParam(name, type, mem, device))
    setParam(name, type, mem, device);
}

void UsdWorld::filterResetParam(const char *name)
{
  resetParam(name);
}

void UsdWorld::commit(UsdDevice* device)
{
  if(!usdBridge)
    return;

  const char* debugName = getName();

  bool isNew = false;
  if(!usdHandle.value)
    isNew = usdBridge->CreateWorld(debugName, usdHandle);

  if (paramChanged || isNew)
  {
    double timeStep = device->getParams().timeStep;
    bool instancesTimeVarying = paramData.timeVarying != 0;

    if (paramData.instances)
    {
      if (paramData.instances->getType() == ANARI_INSTANCE)
      {
        const ANARIInstance* instances = reinterpret_cast<const ANARIInstance*>(paramData.instances->getData());

        uint64_t numInstances = paramData.instances->getLayout().numItems1;
        instanceHandles.resize(numInstances);
        for (uint64_t i = 0; i < numInstances; ++i)
        {
          const UsdInstance* usdInstance = reinterpret_cast<const UsdInstance*>(instances[i]);
          instanceHandles[i] = usdInstance->getUsdHandle();
        }

        if (numInstances)
          usdBridge->SetInstanceRefs(usdHandle, &instanceHandles[0], numInstances, instancesTimeVarying, timeStep);
        else
          usdBridge->DeleteInstanceRefs(usdHandle, instancesTimeVarying, timeStep);
      }
      else
      {
        device->reportStatus(this, ANARI_SPATIAL_FIELD, ANARI_SEVERITY_ERROR, ANARI_STATUS_INVALID_ARGUMENT,
          "UsdWorld '%s' commit failed: 'instance' array elements should be of type ANARI_INSTANCE", debugName);
      }
    }
    else
    {
      usdBridge->DeleteInstanceRefs(usdHandle, instancesTimeVarying, timeStep);
    }

    paramChanged = false;
  }

  usdBridge->SaveScene(); //This needs a better spot.
}