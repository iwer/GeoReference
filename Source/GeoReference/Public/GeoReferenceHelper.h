// Copyright (c) Iwer Petersen. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GDALHelpers.h"

class GEOREFERENCE_API FGeoReferenceHelper
{
public:
    static bool IsWGS84(OGRSpatialReferenceH ref);
    static bool IsUTM(OGRSpatialReferenceH ref);

    /**
    * this code is based on https://www.wavemetrics.com/code-snippet/convert-latitudelongitude-utm
    * which is attributed to Chuck Gantz
    */
    static int GetUTMZone(double Longitude, double Latitude);
    /**
     * This code determines the correct UTM letter designator for the given latitude
     * returns 'Z' if latitude is outside the UTM limits of 84N to 80S
     * Written by Chuck Gantz - chuck.gantz@globalstar.com
     */
    static char GetUTMLetter(double Latitude);

    /**
     * Returns the EPSG number of a UTM zone
     */
    static int GetEPSGForUTM(int utmZone = -1, bool bNorthernHemi = true);
};
