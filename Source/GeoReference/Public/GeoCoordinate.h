// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GeoCoordinate.generated.h"


UENUM(BlueprintType)
enum EGeoCoordinateType {
	GCT_UNDEFINED,
	GCT_UTM,
	GCT_WSG84
};
/**
 * Double precision for geo coordinates. Pure backend type as Blueprint does not understand double
 */
UCLASS()
class GEOREFERENCE_API UGeoCoordinate : public UObject
{
	GENERATED_BODY()
private:
	UPROPERTY()
	double Longitude; // ||
	UPROPERTY()
	double Latitude;  // =
	UPROPERTY()
	TEnumAsByte<EGeoCoordinateType> Type;

public:
	UGeoCoordinate();
	UGeoCoordinate(double longitude, double latitude, EGeoCoordinateType type);

	FVector2D ToFVector2D();
	UGeoCoordinate ToWSG84();
	UGeoCoordinate ToUTM();

};
