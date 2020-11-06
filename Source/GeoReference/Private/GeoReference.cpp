// Copyright (c) Iwer Petersen. All rights reserved.

#include "GeoReference.h"
#include "ROI.h"

FVector FGeoReference::ToGameCoordinate(double Longitude, double Latitude, URegionOfInterest * Region)
{
	UE_LOG(LogTemp, Warning, TEXT("ToGameCoordinate(lon:%f lat:%f region: %s"), Longitude, Latitude, *Region->ToString());
	auto utm = UGeoCoordinate(Longitude,Latitude,EGeoCoordinateType::GCT_WGS84).ToUTM();
	//FVector2D nw_corner_utm(Region->UTMCoordinates.X - Region->SizeM / 2, Region->UTMCoordinates.Y - Region->SizeM / 2);
	// utm reference frame (SW=(UTM_W,UTM_S)) to landscape reference frame (NW=(0,0))
	FVector2D pos = (utm - Region->Location.ToUTM()) * FVector2D(1, -1);
	// scale to landscape (m->cm)
	pos *= 100;
	return FVector(pos.X, pos.Y, 0);
}

bool FGeoReference::IsWGS84(OGRSpatialReferenceH ref)
{
	if (OSRIsGeographic(ref)) {
		auto geocs = OSRGetAttrValue(ref, "GEOGCS", 0);
		if (strncmp(geocs, "WGS 84", 6) == 0) {
			return true;
		}
	}
	return false;
}

bool FGeoReference::IsUTM(OGRSpatialReferenceH ref)
{
	if (OSRIsProjected(ref)) {
		int north;
		int utmzone = OSRGetUTMZone(ref, &north);
		if (utmzone != 0) {
			return true;
		}
	}
	return false;
}

void FGeoReference::GetSize(GDALDatasetRef & gdaldata, double & width, double & height)
{
	GeoTransformRef georef = GDALHelpers::GetGeoTransform(gdaldata);
	FVector2D srcsize = FVector2D(gdaldata->GetRasterXSize(), gdaldata->GetRasterYSize());
	double north = georef[3];
	double west = georef[0];
	double south = georef[3] + srcsize.X * georef[4] + srcsize.Y * georef[5];
	double east = georef[0] + srcsize.X * georef[1] + srcsize.Y * georef[2];

	auto crs = gdaldata->GetProjectionRef();
	if (FGeoReference::IsWGS84(OSRNewSpatialReference(crs))) {
		auto utm_nw = FGeoReference::TransformWGSToUTM(west, north);
		auto utm_se = FGeoReference::TransformWGSToUTM(east, south);

		width = utm_se.Longitude - utm_nw.Longitude;
		height = utm_nw.Latitude - utm_se.Latitude;

	}
	else if (FGeoReference::IsUTM(OSRNewSpatialReference(crs))) {
		width = (east - west);
		height = (north - south);
	}
	else {
		width = srcsize.X;
		height = srcsize.Y;
	}
}

// this code is based on https://www.wavemetrics.com/code-snippet/convert-latitudelongitude-utm
// which is attributed to Chuck Gantz

int FGeoReference::UTMZone(double longitude, double latitude)
{
	int utmzone = floor((longitude + 180) / 6) + 1;

	if (latitude >= 56.0 && latitude < 64.0 && longitude >= 3.0 && longitude < 12.0)
		utmzone = 32;

	// Special zones for Svalbard
	if (latitude >= 72.0 && latitude < 84.0) {
		if (longitude >= 0.0 && longitude < 9.0) {
			utmzone = 31;
		}
		else if (longitude >= 9.0 && longitude < 21.0) {
			utmzone = 33;
		}
		else if (longitude >= 21.0 && longitude < 33.0) {
			utmzone = 35;
		}
		else if (longitude >= 33.0 && longitude < 42.0) {
			utmzone = 37;
		}
	}
	return utmzone;
}

// This code determines the correct UTM letter designator for the given latitude
// returns 'Z' if latitude is outside the UTM limits of 84N to 80S
// Written by Chuck Gantz- chuck.gantz@globalstar.com

char FGeoReference::UTMLetter(double latitude)
{
	char LetterDesignator;

	if ((84 >= latitude) && (latitude >= 72)) LetterDesignator = 'X';
	else if ((72 > latitude) && (latitude >= 64)) LetterDesignator = 'W';
	else if ((64 > latitude) && (latitude >= 56)) LetterDesignator = 'V';
	else if ((56 > latitude) && (latitude >= 48)) LetterDesignator = 'U';
	else if ((48 > latitude) && (latitude >= 40)) LetterDesignator = 'T';
	else if ((40 > latitude) && (latitude >= 32)) LetterDesignator = 'S';
	else if ((32 > latitude) && (latitude >= 24)) LetterDesignator = 'R';
	else if ((24 > latitude) && (latitude >= 16)) LetterDesignator = 'Q';
	else if ((16 > latitude) && (latitude >= 8)) LetterDesignator = 'P';
	else if ((8 > latitude) && (latitude >= 0)) LetterDesignator = 'N';
	else if ((0 > latitude) && (latitude >= -8)) LetterDesignator = 'M';
	else if ((-8> latitude) && (latitude >= -16)) LetterDesignator = 'L';
	else if ((-16 > latitude) && (latitude >= -24)) LetterDesignator = 'K';
	else if ((-24 > latitude) && (latitude >= -32)) LetterDesignator = 'J';
	else if ((-32 > latitude) && (latitude >= -40)) LetterDesignator = 'H';
	else if ((-40 > latitude) && (latitude >= -48)) LetterDesignator = 'G';
	else if ((-48 > latitude) && (latitude >= -56)) LetterDesignator = 'F';
	else if ((-56 > latitude) && (latitude >= -64)) LetterDesignator = 'E';
	else if ((-64 > latitude) && (latitude >= -72)) LetterDesignator = 'D';
	else if ((-72 > latitude) && (latitude >= -80)) LetterDesignator = 'C';
	else LetterDesignator = 'Z'; //This is here as an error flag to show that the Latitude is outside the UTM limits

	return LetterDesignator;
}

UGeoCoordinate FGeoReference::TransformWGSToUTM(double longitude, double latitude)
{
	OGRSpatialReference sourceSRS;
	sourceSRS.SetWellKnownGeogCS("WGS84");

	OGRSpatialReference targetSRS;

	int utmzone = FGeoReference::UTMZone(longitude, latitude);
	targetSRS.SetUTM(utmzone, latitude >= 0);

	OGRPoint point(longitude, latitude);
	// if latitude is bigger than 84 or smaller than -80 there is no utm zone
	if (FGeoReference::UTMLetter(latitude) == 'Z') {
		UE_LOG(LogTemp, Error, TEXT("FGeoReference: latitude %f is outside defined UTM Zones!"), latitude);
		return UGeoCoordinate();
	}

	point.assignSpatialReference(&sourceSRS);
	point.transformTo(&targetSRS);
    return UGeoCoordinate(point.getX(), point.getY(), EGeoCoordinateType::GCT_UTM, utmzone, latitude >= 0);


}

UGeoCoordinate FGeoReference::TransformUTMToWGS(double longitude, double latitude, int utmzone, bool north)
{
	OGRSpatialReference sourceSRS;
	sourceSRS.SetUTM(utmzone, north);

	OGRSpatialReference targetSRS;
	targetSRS.SetWellKnownGeogCS("WGS84");

	OGRPoint point(longitude, latitude);
	point.assignSpatialReference(&sourceSRS);
	point.transformTo(&targetSRS);
	return UGeoCoordinate(point.getX(), point.getY(), EGeoCoordinateType::GCT_WGS84);
}
