// Copyright 2020 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "UsdSampler.h"
#include "UsdBridge/UsdBridge.h"
#include "UsdDevice.h"

DEFINE_PARAMETER_MAP(UsdSampler, 
  REGISTER_PARAMETER_MACRO("name", ANARI_STRING, name)
  REGISTER_PARAMETER_MACRO("usd::name", ANARI_STRING, usdName)
  REGISTER_PARAMETER_MACRO("usd::timestep", ANARI_FLOAT64, timeStep)
  REGISTER_PARAMETER_MACRO("usd::timevarying", ANARI_INT32, timeVarying)
  REGISTER_PARAMETER_MACRO("filename", ANARI_STRING, fileName)
  REGISTER_PARAMETER_MACRO("wrapMode1", ANARI_STRING, wrapS)
  REGISTER_PARAMETER_MACRO("wrapMode2", ANARI_STRING, wrapT)
)

namespace
{ 
  UsdBridgeSamplerData::WrapMode ANARIToUsdBridgeWrapMode(const char* anariWrapMode)
  {
    UsdBridgeSamplerData::WrapMode usdWrapMode = UsdBridgeSamplerData::WrapMode::BLACK;
    if (!std::strcmp(anariWrapMode, "clampToEdge"))
    {
      usdWrapMode = UsdBridgeSamplerData::WrapMode::CLAMP;
    }
    else if (!std::strcmp(anariWrapMode, "repeat"))
    {
      usdWrapMode = UsdBridgeSamplerData::WrapMode::REPEAT;
    }
    else if (!std::strcmp(anariWrapMode, "mirrorRepeat"))
    {
      usdWrapMode = UsdBridgeSamplerData::WrapMode::MIRROR;
    }
    return usdWrapMode;
  }
}

UsdSampler::UsdSampler(const char* name, UsdBridge* bridge)
  : BridgedBaseObjectType(ANARI_SAMPLER, name, bridge)
{
}

UsdSampler::~UsdSampler()
{
#ifdef OBJECT_LIFETIME_EQUALS_USD_LIFETIME
  if(usdBridge)
    usdBridge->DeleteSampler(usdHandle);
#endif
}

void UsdSampler::filterSetParam(const char *name,
  ANARIDataType type,
  const void *mem,
  UsdDevice* device)
{
  if (filterNameParam(name, type, mem, device))
    setParam(name, type, mem, device);
}

void UsdSampler::filterResetParam(const char *name)
{
  resetParam(name);
}

void UsdSampler::commit(UsdDevice* device)
{
  if(!usdBridge)
    return;

  bool isNew = false;
  if (!usdHandle.value)
    isNew = usdBridge->CreateSampler(getName(), usdHandle);

  if (paramChanged || isNew)
  {
    UsdBridgeSamplerData samplerData;
    samplerData.FileName = paramData.fileName;
    samplerData.WrapS = ANARIToUsdBridgeWrapMode(paramData.wrapS);
    samplerData.WrapT = ANARIToUsdBridgeWrapMode(paramData.wrapT);

    samplerData.TimeVarying = (UsdBridgeSamplerData::DataMemberId)paramData.timeVarying;

    usdBridge->SetSamplerData(usdHandle, samplerData, paramData.timeStep);

    paramChanged = false;
  }
}
