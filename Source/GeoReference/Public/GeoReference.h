// Copyright (c) Iwer Petersen. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GDALHelpers.h"

class URegionOfInterest;

class GEOREFERENCE_API FGeoReference
{
public:
	static FVector ToGameCoordinate(double Longitude, double Latitude, URegionOfInterest * Region);

	static bool IsWGS84(OGRSpatialReferenceH ref);
	static bool IsUTM(OGRSpatialReferenceH ref);

	static void GetSize(GDALDatasetRef &gdaldata, double &width, double &height);

	static int UTMZone(double longitude, double latitude);
	static char UTMLetter(double latitude);

	static UGeoCoordinate TransformWGSToUTM(double longitude, double latitude);
	static UGeoCoordinate TransformUTMToWGS(double longitude, double latitude, int utmzone, bool north);


};
