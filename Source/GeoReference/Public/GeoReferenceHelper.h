// Copyright (c) Iwer Petersen. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GDALHelpers.h"

class GEOREFERENCE_API FGeoReferenceHelper
{
public:
	static bool IsWGS84(OGRSpatialReferenceH ref);
	static bool IsUTM(OGRSpatialReferenceH ref);
};
