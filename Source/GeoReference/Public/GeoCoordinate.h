// Copyright (c) Iwer Petersen. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
// #include "GeoCoordinate.generated.h"

class URegionOfInterest;

/**
 * Double precision for geo coordinates. Pure backend type as Blueprint does not
 * understand double. Internally always in WGS84 format
 */
class GEOREFERENCE_API UGeoCoordinate
{
public:
    static constexpr int EPSG_WGS84 = 4326;

    double Longitude; // ||
    double Latitude;  // ==

    UGeoCoordinate();
    UGeoCoordinate(double longitude, double latitude, int EPSGNumber = -1);

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

    /**
    * Converts GeoCoordinate in its current representation to a FVector2D.
    */
    FVector2D ToFVector2D();
    FVector2D ToFVector2DInUTM();
    FVector2D ToFVector2DInEPSG(int EPSGNumber);

    /**
     * Converts GeoCoordinate to GameCoordinate relative to ROI Center. The GeoCoordinate is transformed to the UTM
     * reference frame of the ROI. Then the difference between the ROI center UTM coordinates and the GeoCoordinate is
     * taken and multiplied by 100 to get game coordinates in cm.
     */
    FVector ToGameCoordinate(URegionOfInterest &ROI);

private:
    struct DVector2D {
        double X;
        double Y;
        DVector2D()
        {
            X = 0;
            Y = 0;
        }
        DVector2D(double x, double y)
        {
            X = x;
            Y = y;
        }
    };

    DVector2D Add(UGeoCoordinate &other, int EPSGNumber);
    DVector2D Subtract(UGeoCoordinate &other, int EPSGNumber);
};
