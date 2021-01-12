// Copyright (c) Iwer Petersen. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GDALHelpers.h"
#include "GeoCoordinate.h"

#include "ROI.generated.h"

UENUM(BlueprintType)
enum EROICorner {
	NW,
	SW,
	NE,
	SE
};

UENUM(BlueprintType)
enum EROIBorder {
	North,
	South,
	West,
	East
};

/**
 * Defines a region of interest in geo coordinates
 * The region is defined in a projected coordinate system with
 * linear scaled x and y axis, which are orthogonal. A WSG84
 * coordinate defines the center of the region (and is
 * calculated to UTM). As UTM coordinates are in meter and have
 * orthogonal axis corner coordinates are calculated using the
 * size (+/-(size/2))
 */
UCLASS(BlueprintType)
class GEOREFERENCE_API URegionOfInterest : public UObject
{
	GENERATED_BODY()
public:
    /**
    *
    */
	UGeoCoordinate Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROI", Meta = (ExposeOnSpawn = "true"))
	FVector2D WGS84Coordinates;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "ROI")
	FVector2D UTMCoordinates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ROI", Meta = (ExposeOnSpawn = "true"))
	float SizeM;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "ROI")
	int UTMZone;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "ROI")
	bool bNorthernHemisphere;


	URegionOfInterest();
	URegionOfInterest(FVector2D geocoordinates, float size);
	URegionOfInterest(GDALDatasetRef gdaldata);
    URegionOfInterest(const char * crsString, double east, double west, double north, double south);
	~URegionOfInterest();

    static void GetSize(GDALDatasetRef &gdaldata, double &OutWidth, double &OutHeight);


    UFUNCTION(BlueprintCallable, Category = "ROI|Initializer")
	void Init(FVector2D geocoordinates, float size);
	void InitFromGDAL(GDALDatasetRef &gdaldata);
    void InitFromCRSAndEdges(const char * crsString, double east, double west, double north, double south);

	UFUNCTION(BlueprintCallable, Category = "ROI|Methods")
	FVector2D GetCorner(EROICorner corner, EGeoCoordinateType coordinatetype);

	UFUNCTION(BlueprintCallable, Category = "ROI|Methods")
	float GetBorder(EROIBorder border, EGeoCoordinateType coordinatetype);


    bool Surrounds(UGeoCoordinate &coord);


	FString ToString();
};
