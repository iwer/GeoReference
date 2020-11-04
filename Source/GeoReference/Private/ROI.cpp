// Fill out your copyright notice in the Description page of Project Settings.


#include "ROI.h"
#include "GeoReference.h"
#include "..\Public\ROI.h"



URegionOfInterest::URegionOfInterest()
	: URegionOfInterest(FVector2D(),0)
{
}

URegionOfInterest::URegionOfInterest(FVector2D geocoordinates, float size)
	: WSG84Coordinates(geocoordinates)
	, UTMCoordinates(FVector2D())
	, SizeM(size)
	, UTMZone(-1)
	, NorthernHemisphere(true)
{
	Init(geocoordinates, size);
}

URegionOfInterest::URegionOfInterest(GDALDatasetRef gdaldata)
	: URegionOfInterest()
{
	InitFromGDAL(gdaldata);
}

URegionOfInterest::~URegionOfInterest()
{
}

void URegionOfInterest::Init(FVector2D geocoordinates, float size)
{
	WSG84Coordinates = geocoordinates;
	UTMCoordinates = FGeoReference::TransformWGSToUTM(WSG84Coordinates.X, WSG84Coordinates.Y);

	UTMZone = FGeoReference::UTMZone(WSG84Coordinates.X, WSG84Coordinates.Y);
	NorthernHemisphere = WSG84Coordinates.Y >= 0;

	UE_LOG(LogTemp, Warning, TEXT("ROI: WSG: %s UTM: %s"), *WSG84Coordinates.ToString(), *UTMCoordinates.ToString());
}

void URegionOfInterest::InitFromGDAL(GDALDatasetRef &gdaldata)
{
	GeoTransformRef georef = GDALHelpers::GetGeoTransform(gdaldata);
	FVector2D srcsize = FVector2D(gdaldata->GetRasterXSize(), gdaldata->GetRasterYSize());
	double north = georef[3];
	double west = georef[0];
	double south = north + srcsize.X * georef[4] + srcsize.Y * georef[5];
	double east = west + srcsize.X * georef[1] + srcsize.Y * georef[2];

	auto crs_s = gdaldata->GetProjectionRef();
	auto crs = OSRNewSpatialReference(crs_s);
	if (FGeoReference::IsWGS84(crs)) {
		WSG84Coordinates.X = (east - west) / 2 + west;
		WSG84Coordinates.Y = (north - south) / 2 + south;

		UTMCoordinates = FGeoReference::TransformWGSToUTM(WSG84Coordinates.X, WSG84Coordinates.Y);

		auto utm_west_center = FGeoReference::TransformWGSToUTM(west, WSG84Coordinates.Y);
		auto utm_east_center = FGeoReference::TransformWGSToUTM(east, WSG84Coordinates.Y);
		auto utm_north_center = FGeoReference::TransformWGSToUTM(WSG84Coordinates.X, north);
		auto utm_south_center = FGeoReference::TransformWGSToUTM(WSG84Coordinates.X, south);

		SizeM = std::min(utm_east_center.X - utm_west_center.X, utm_south_center.Y - utm_north_center.Y);

	}
	else if (FGeoReference::IsUTM(crs)) {
		int northhemi;
		UTMZone = OSRGetUTMZone(crs, &northhemi);
		NorthernHemisphere = (northhemi == TRUE);
		UTMCoordinates = FVector2D((east - west) / 2 + west, (north - south) / 2 + south);
		WSG84Coordinates = FGeoReference::TransformUTMToWGS((east - west) / 2 + west, (north - south) / 2 + south, UTMZone, NorthernHemisphere);

		SizeM = std::min((east - west), (south - north));
	}
}

FVector2D URegionOfInterest::GetCorner(EROICorner corner, EROICoordinateType coordinatetype)
{

	float UTMWest = UTMCoordinates.X - SizeM / 2;
	float UTMEast = UTMCoordinates.X + SizeM / 2;
	float UTMNorth = UTMCoordinates.Y + SizeM / 2;
	float UTMSouth = UTMCoordinates.Y - SizeM / 2;


	FVector2D cornerCoords;
	if (corner == EROICorner::NW) {
		cornerCoords.X = UTMWest;
		cornerCoords.Y = UTMNorth;
	}
	else if (corner == EROICorner::SW) {
		cornerCoords.X = UTMWest;
		cornerCoords.Y = UTMSouth;
	}
	else if (corner == EROICorner::NE) {
		cornerCoords.X = UTMEast;
		cornerCoords.Y = UTMNorth;
	}
	else if (corner == EROICorner::SE) {
		cornerCoords.X = UTMEast;
		cornerCoords.Y = UTMSouth;
	}

	if (coordinatetype == EROICoordinateType::WSG84) {
		auto wgsc = FGeoReference::TransformUTMToWGS(cornerCoords.X, cornerCoords.Y, UTMZone, NorthernHemisphere);
		return wgsc;
	}

	return cornerCoords;
}

float URegionOfInterest::GetBorder(EROIBorder border, EROICoordinateType coordinatetype)
{
	float UTMWest = UTMCoordinates.X - SizeM / 2;
	float UTMEast = UTMCoordinates.X + SizeM / 2;
	float UTMNorth = UTMCoordinates.Y + SizeM / 2;
	float UTMSouth = UTMCoordinates.Y - SizeM / 2;


	float bordercoord = 0.0;
	if (coordinatetype == EROICoordinateType::UTM) {
		if (border == EROIBorder::North) {
			bordercoord = UTMNorth;
		}
		else if (border == EROIBorder::South) {
			bordercoord = UTMSouth;
		}
		else if (border == EROIBorder::West) {
			bordercoord = UTMWest;
		}
		else if (border == EROIBorder::East) {
			bordercoord = UTMEast;
		}
	}
	else {
		// Build Utm coordinates for center of roi edges
		FVector2D borderCoords;
		if (border == EROIBorder::North) {
			borderCoords.X = UTMCoordinates.X;
			borderCoords.Y = UTMNorth;
		}
		else if (border == EROIBorder::South) {
			borderCoords.X = UTMCoordinates.X;
			borderCoords.Y = UTMSouth;
		}
		else if (border == EROIBorder::West) {
			borderCoords.X = UTMEast;
			borderCoords.Y = UTMCoordinates.Y;
		}
		else if (border == EROIBorder::East) {
			borderCoords.X = UTMEast;
			borderCoords.Y = UTMCoordinates.Y;
		}
		//transform to wsg84
		auto wgsc = FGeoReference::TransformUTMToWGS(borderCoords.X, borderCoords.Y, UTMZone, NorthernHemisphere);
		// select x or y
		if (border == EROIBorder::West || border == EROIBorder::East) {
			bordercoord = borderCoords.X;
		}
		else {
			bordercoord = borderCoords.Y;
		}
	}
	return bordercoord;
}

FString URegionOfInterest::ToString()
{
	return TEXT("WSG85:") + WSG84Coordinates.ToString() + TEXT("; UTM:") + UTMCoordinates.ToString()
		+ TEXT(";Size:") + FString::SanitizeFloat(SizeM) + TEXT(";UTMZone:") + FString::FromInt(UTMZone)
		+ TEXT(";NorthHemi:") + (NorthernHemisphere ? TEXT("true") : TEXT("false"));
}


