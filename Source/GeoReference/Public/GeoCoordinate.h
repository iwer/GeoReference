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
    bool NorthernHemisphere;

	UGeoCoordinate();
	UGeoCoordinate(double longitude, double latitude, EGeoCoordinateType type, int utmZone = -1, bool northernHemi = true);

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
	UGeoCoordinate ToUTM(int utmZone = -1, bool north = true);

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
};
