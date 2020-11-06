// Fill out your copyright notice in the Description page of Project Settings.

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

	FVector2D ToFVector2D();
	UGeoCoordinate ToWGS84();
	UGeoCoordinate ToUTM();

    UGeoCoordinate operator+(const UGeoCoordinate & other);
    UGeoCoordinate operator-(const UGeoCoordinate & other);

    FVector2D operator*(const FVector2D & other);

};
