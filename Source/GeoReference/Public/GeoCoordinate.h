// Copyright (c) Iwer Petersen. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GeoCoordinate.generated.h"


UENUM(BlueprintType)
enum EGeoCoordinateType {
	GCT_UNDEFINED,
	GCT_UTM,
	GCT_WGS84
};

class URegionOfInterest;

/**
 * Double precision for geo coordinates. Pure backend type as Blueprint does not understand double
 */
class GEOREFERENCE_API UGeoCoordinate
{
public:
    double Longitude; // ||
	double Latitude;  // =
	TEnumAsByte<EGeoCoordinateType> Type;
    int UTMZone;
    bool bNorthernHemisphere;

	UGeoCoordinate();
	UGeoCoordinate(double longitude, double latitude, EGeoCoordinateType type, int utmZone = -1, bool bNorthernHemi = true);

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
    * Converts GeoCoordinate in its current representation to a FVector2D.
    */
	FVector2D ToFVector2D();
    /**
    * Converts GeoCoordinate to WGS84 ( Longitude / Latitude in degree)
    */
	UGeoCoordinate ToWGS84();
    /**
    * Converts GeoCordinate to UTM. Defaults to home utm zone, but can be overriden
    */
	UGeoCoordinate ToUTM(int utmZone = -1, bool bNorth = true);

	/**
	 * Converts GeoCoordinate to GameCoordinate relative to ROI Center. The GeoCoordinate is transformed to the UTM
	 * reference frame of the ROI. Then the difference between the ROI center UTM coordinates and the GeoCoordinate is
	 * taken and multiplied by 100 to get game coordinates in cm.
	 */
    FVector ToGameCoordinate(URegionOfInterest &ROI);

    /**
    * Adds two coordinates of the same type (and the same zone in case of UTM)
    */
    UGeoCoordinate operator+(const UGeoCoordinate & other);
    /**
    * Subracts two coordinates of the same type (and the same zone in case of UTM)
    */
    UGeoCoordinate operator-(const UGeoCoordinate & other);

    /**
    * Returns a component-wise scaled FVector2D
    */
    FVector2D operator*(const FVector2D & other);

private:
    /**
     * Transforms the coordinate to UTM returning a new object, optionally target utm zone and hemisphere can be used
     * instead of coordinates zone.
     */
    UGeoCoordinate TransformToUTM(int TargetUTMZone = -1, bool bTargetNorthernHemi = true);
    /**
     * Transforms the coordinate to WGS84 returning a new object.
     */
    UGeoCoordinate TransformToWGS84();
};
