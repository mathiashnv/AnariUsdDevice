// Copyright 2020 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "UsdSpatialField.h"
#include "UsdBridge/UsdBridge.h"
#include "UsdDataArray.h"
#include "UsdAnari.h"
#include "UsdDevice.h"

DEFINE_PARAMETER_MAP(UsdSpatialField,
  REGISTER_PARAMETER_MACRO("name", ANARI_STRING, name)
  REGISTER_PARAMETER_MACRO("usd::name", ANARI_STRING, usdName)
  REGISTER_PARAMETER_MACRO("usd::timestep", ANARI_FLOAT64, timeStep)
  REGISTER_PARAMETER_MACRO("usd::timevarying", ANARI_INT32, timeVarying)
  REGISTER_PARAMETER_MACRO("data", ANARI_ARRAY, data)
  REGISTER_PARAMETER_MACRO("spacing", ANARI_FLOAT32_VEC3, gridSpacing)
  REGISTER_PARAMETER_MACRO("origin", ANARI_FLOAT32_VEC3, gridOrigin)
) // See .h for usage.

UsdSpatialField::UsdSpatialField(const char* name, const char* type, UsdBridge* bridge)
  : BridgedBaseObjectType(ANARI_SPATIAL_FIELD, name, bridge)
{
}

UsdSpatialField::~UsdSpatialField()
{
#ifdef OBJECT_LIFETIME_EQUALS_USD_LIFETIME
  if(usdBridge)
    usdBridge->DeleteSpatialField(usdHandle);
#endif
}

void UsdSpatialField::filterSetParam(
  const char *name,
  ANARIDataType type,
  const void *mem,
  UsdDevice* device)
{
  if(filterNameParam(name, type, mem, device))
    setParam(name, type, mem, device);
}

void UsdSpatialField::filterResetParam(const char *name)
{
  resetParam(name);
}

void UsdSpatialField::commit(UsdDevice* device)
{
  if(!usdBridge)
    return;

  const char* debugName = getName();

  bool isNew = false;
  if(!usdHandle.value)
    isNew = usdBridge->CreateSpatialField(debugName, usdHandle);

  // Only perform type checks, actual data gets uploaded during UsdVolume::commit()
  const UsdDataArray* fieldDataArray = this->paramData.data;
  if (!fieldDataArray)
  {
    device->reportStatus(this, ANARI_SPATIAL_FIELD, ANARI_SEVERITY_ERROR, ANARI_STATUS_INVALID_OPERATION,
      "UsdSpatialField '%s' commit failed: data missing.", debugName);
    return;
  }

  const UsdDataLayout& dataLayout = fieldDataArray->getLayout();
  if (AssertNoStride(dataLayout, device, debugName, "data"))
    return;

  switch (fieldDataArray->getType())
  {
  case ANARI_INT8:
  case ANARI_UINT8:
  case ANARI_INT16:
  case ANARI_UINT16:
  case ANARI_INT32:
  case ANARI_UINT32:
  case ANARI_INT64:
  case ANARI_UINT64:
  case ANARI_FLOAT32:
  case ANARI_FLOAT64:
    break;
  default:
    device->reportStatus(this, ANARI_SPATIAL_FIELD, ANARI_SEVERITY_ERROR, ANARI_STATUS_INVALID_ARGUMENT,
      "UsdSpatialField '%s' commit failed: incompatible data type.", debugName);
    return;
  }

  // Make sure that parameters are set a first time
  paramChanged = paramChanged || isNew;

  // Handled in UsdVolume
  //if (paramChanged)
  //{
  //  if (paramData.data)
  //  {
  //
  //
  //
  //  }
  //  else
  //  {
  //    if (!paramData.data)
  //      device->reportStatus(this, ANARI_SPATIAL_FIELD, ANARI_SEVERITY_ERROR, ANARI_STATUS_INVALID_ARGUMENT, "UsdSpatialField '%s' commit failed: missing 'data'.", debugName);
  //  }
  //
  //  paramChanged = false;
  //}
}