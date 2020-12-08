// Fill out your copyright notice in the Description page of Project Settings.


#include "ROI.h"
#include "GeoReference.h"


URegionOfInterest::URegionOfInterest()
	: URegionOfInterest(FVector2D(),0)
{
}

URegionOfInterest::URegionOfInterest(FVector2D geocoordinates, float size)
	: Location()
    , WGS84Coordinates()
	, UTMCoordinates()
	, SizeM(0)
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
    Location = UGeoCoordinate(geocoordinates.X, geocoordinates.Y, EGeoCoordinateType::GCT_WGS84);
	WGS84Coordinates = Location.ToFVector2D();
	UTMCoordinates = Location.ToUTM().ToFVector2D();

	UTMZone = FGeoReference::UTMZone(geocoordinates.X, geocoordinates.Y);
	NorthernHemisphere = (geocoordinates.Y >= 0);
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
        Location = UGeoCoordinate((east - west) / 2 + west, (north - south) / 2 + south, EGeoCoordinateType::GCT_WGS84);
		WGS84Coordinates = Location.ToFVector2D();
		UTMCoordinates = Location.ToUTM().ToFVector2D();

		auto utm_west_center = UGeoCoordinate(west, WGS84Coordinates.Y, EGeoCoordinateType::GCT_WGS84).ToUTM();
		auto utm_east_center = UGeoCoordinate(east, WGS84Coordinates.Y, EGeoCoordinateType::GCT_WGS84).ToUTM();
		auto utm_north_center = UGeoCoordinate(WGS84Coordinates.X, north, EGeoCoordinateType::GCT_WGS84).ToUTM();
		auto utm_south_center = UGeoCoordinate(WGS84Coordinates.X, south, EGeoCoordinateType::GCT_WGS84).ToUTM();

		SizeM = std::min(utm_east_center.Longitude - utm_west_center.Longitude, utm_south_center.Latitude - utm_north_center.Latitude);

	}
	else if (FGeoReference::IsUTM(crs)) {
		int northhemi;
		UTMZone = OSRGetUTMZone(crs, &northhemi);
		NorthernHemisphere = (northhemi == TRUE);
        Location = UGeoCoordinate((east - west) / 2 + west, (north - south) / 2 + south, EGeoCoordinateType::GCT_UTM);
		UTMCoordinates = Location.ToFVector2D();
		WGS84Coordinates = Location.ToWGS84().ToFVector2D();

		SizeM = std::min((east - west), (south - north));
		UE_LOG(LogTemp, Warning, TEXT("URegionOfInterest: Zone: %d Loc: %f:%f UTM: %f:%f WGS: %f:%f"), UTMZone, Location.Longitude, Location.Latitude, UTMCoordinates.X, UTMCoordinates.Y, WGS84Coordinates.X, WGS84Coordinates.Y);
	} else {
		UE_LOG(LogTemp, Warning, TEXT("URegionOfInterest: Unknown CRS: %s"), crs_s);
	}

}

FVector2D URegionOfInterest::GetCorner(EROICorner corner, EGeoCoordinateType coordinatetype)
{
	UGeoCoordinate cornerCoords;
    if (coordinatetype == EGeoCoordinateType::GCT_UNDEFINED) {
        return FVector2D();
    }

	if (corner == EROICorner::NW) {
		cornerCoords = Location + UGeoCoordinate(-SizeM, -SizeM, coordinatetype);
	}
	else if (corner == EROICorner::SW) {
		cornerCoords = Location + UGeoCoordinate(-SizeM, SizeM, coordinatetype);
	}
	else if (corner == EROICorner::NE) {
		cornerCoords = Location + UGeoCoordinate(SizeM, -SizeM, coordinatetype);
	}
	else if (corner == EROICorner::SE) {
		cornerCoords = Location + UGeoCoordinate(SizeM, SizeM, coordinatetype);
	}

	if (coordinatetype == EGeoCoordinateType::GCT_WGS84) {
		cornerCoords = cornerCoords.ToWGS84();
	}

	return cornerCoords.ToFVector2D();
}

float URegionOfInterest::GetBorder(EROIBorder border, EGeoCoordinateType coordinatetype)
{
	float UTMWest = UTMCoordinates.X - SizeM / 2;
	float UTMEast = UTMCoordinates.X + SizeM / 2;
	float UTMNorth = UTMCoordinates.Y + SizeM / 2;
	float UTMSouth = UTMCoordinates.Y - SizeM / 2;


	float bordercoord = 0.0;
	if (coordinatetype == EGeoCoordinateType::GCT_UTM) {
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
	else if (coordinatetype == EGeoCoordinateType::GCT_WGS84){
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
	return TEXT("WSG85:") + WGS84Coordinates.ToString() + TEXT("; UTM:") + UTMCoordinates.ToString()
		+ TEXT(";Size:") + FString::SanitizeFloat(SizeM) + TEXT(";UTMZone:") + FString::FromInt(UTMZone)
		+ TEXT(";NorthHemi:") + (NorthernHemisphere ? TEXT("true") : TEXT("false"));
}
