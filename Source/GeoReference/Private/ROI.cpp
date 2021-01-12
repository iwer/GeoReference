// Copyright (c) Iwer Petersen. All rights reserved.


#include "ROI.h"
#include "GeoReferenceHelper.h"


URegionOfInterest::URegionOfInterest()
	: URegionOfInterest(FVector2D(),0)
{
}

URegionOfInterest::URegionOfInterest(FVector2D geocoordinates, float size)
	: Location(geocoordinates.X, geocoordinates.Y, EGeoCoordinateType::GCT_WGS84)
    , WGS84Coordinates(geocoordinates)
	, UTMCoordinates()
	, SizeM(size)
	, UTMZone(-1)
	, bNorthernHemisphere(true)
{
	Init(geocoordinates, size);
}

URegionOfInterest::URegionOfInterest(GDALDatasetRef gdaldata)
	: URegionOfInterest()
{
	InitFromGDAL(gdaldata);
}

URegionOfInterest::URegionOfInterest(const char * crsString, double east, double west, double north, double south)
    : URegionOfInterest()
{
    InitFromCRSAndEdges(crsString, east, west, north, south);
}
URegionOfInterest::~URegionOfInterest()
{
}

void URegionOfInterest::GetSize(GDALDatasetRef &gdaldata, double &OutWidth, double &OutHeight)
{
    GeoTransformRef georef = GDALHelpers::GetGeoTransform(gdaldata);
    FVector2D srcsize = FVector2D(gdaldata->GetRasterXSize(), gdaldata->GetRasterYSize());
    double north = georef[3];
    double west = georef[0];
    double south = georef[3] + srcsize.X * georef[4] + srcsize.Y * georef[5];
    double east = georef[0] + srcsize.X * georef[1] + srcsize.Y * georef[2];

    auto crs = gdaldata->GetProjectionRef();
    if (FGeoReferenceHelper::IsWGS84(OSRNewSpatialReference(crs))) {
        auto approxCenter = UGeoCoordinate(west + (east - west) / 2,
                                          south + (north - south) / 2,
                                          EGeoCoordinateType::GCT_WGS84)
                                                  .ToUTM();
        auto utm_nw = UGeoCoordinate(west, north, EGeoCoordinateType::GCT_WGS84).ToUTM(approxCenter.UTMZone, approxCenter.bNorthernHemisphere);
        auto utm_ne = UGeoCoordinate(east, north, EGeoCoordinateType::GCT_WGS84).ToUTM(approxCenter.UTMZone, approxCenter.bNorthernHemisphere);
        auto utm_sw = UGeoCoordinate(west, south, EGeoCoordinateType::GCT_WGS84).ToUTM(approxCenter.UTMZone, approxCenter.bNorthernHemisphere);

        OutWidth = utm_ne.Longitude - utm_nw.Longitude;
        OutHeight = utm_nw.Latitude - utm_sw.Latitude;
    }
    else if (FGeoReferenceHelper::IsUTM(OSRNewSpatialReference(crs))) {
        OutWidth = (east - west);
        OutHeight = (north - south);
    }
    else {
        OutWidth = srcsize.X;
        OutHeight = srcsize.Y;
    }
}

void URegionOfInterest::Init(FVector2D geocoordinates, float size)
{
    // UE_LOG(LogTemp, Warning, TEXT("URegionOfInterest: Init from %s; %f m"), *geocoordinates.ToString(), size);
    Location = UGeoCoordinate(geocoordinates.X, geocoordinates.Y, EGeoCoordinateType::GCT_WGS84);
    auto UTMLoc = Location.ToUTM();
    SizeM = size;
    WGS84Coordinates = geocoordinates;
    UTMCoordinates = UTMLoc.ToFVector2D();

	UTMZone = UTMLoc.UTMZone;
    bNorthernHemisphere = UTMLoc.bNorthernHemisphere;

    // UE_LOG(LogTemp, Warning, TEXT("URegionOfInterest: After Init: %s"), *ToString())
}

void URegionOfInterest::InitFromGDAL(GDALDatasetRef &gdaldata)
{
	GeoTransformRef georef = GDALHelpers::GetGeoTransform(gdaldata);
	FVector2D srcsize = FVector2D(gdaldata->GetRasterXSize(), gdaldata->GetRasterYSize());
	double north = georef[3];
	double west = georef[0];
	double south = north + srcsize.X * georef[4] + srcsize.Y * georef[5];
	double east = west + srcsize.X * georef[1] + srcsize.Y * georef[2];

	const char * crs_s = gdaldata->GetProjectionRef();

    InitFromCRSAndEdges(crs_s, east, west, north, south);
}

void URegionOfInterest::InitFromCRSAndEdges(const char * crsString, double east, double west, double north, double south)
{
    OGRSpatialReferenceH crs = OSRNewSpatialReference(crsString);
    if (FGeoReferenceHelper::IsWGS84(crs)) {
        // Location = UGeoCoordinate((east - west) / 2 + west, (north - south) / 2 + south, EGeoCoordinateType::GCT_WGS84);
        // WGS84Coordinates = Location.ToFVector2D();
        // UTMCoordinates = Location.ToUTM().ToFVector2D();
        //
        // auto utm_west_center = UGeoCoordinate(west, WGS84Coordinates.Y, EGeoCoordinateType::GCT_WGS84).ToUTM();
        // auto utm_east_center = UGeoCoordinate(east, WGS84Coordinates.Y, EGeoCoordinateType::GCT_WGS84).ToUTM();
        // auto utm_north_center = UGeoCoordinate(WGS84Coordinates.X, north, EGeoCoordinateType::GCT_WGS84).ToUTM();
        // auto utm_south_center = UGeoCoordinate(WGS84Coordinates.X, south, EGeoCoordinateType::GCT_WGS84).ToUTM();

        // rough (wgs) center for utmzone calculation
        UTMZone = UGeoCoordinate::GetUTMZone(west + (east - west), south + (north - south));
        bNorthernHemisphere = (south + (north - south) >= 0);


        auto utm_nw = UGeoCoordinate(west, north, EGeoCoordinateType::GCT_WGS84).ToUTM();
        auto utm_sw = UGeoCoordinate(west, south, EGeoCoordinateType::GCT_WGS84).ToUTM();
        auto utm_ne = UGeoCoordinate(east, north, EGeoCoordinateType::GCT_WGS84).ToUTM();
        auto utm_se = UGeoCoordinate(east, south, EGeoCoordinateType::GCT_WGS84).ToUTM();

        UGeoCoordinate center(utm_ne.Longitude - utm_nw.Longitude, utm_nw.Latitude - utm_sw.Latitude, EGeoCoordinateType::GCT_UTM, UTMZone, bNorthernHemisphere);

        Location = center;
        WGS84Coordinates = Location.ToFVector2D();
        UTMCoordinates = Location.ToUTM().ToFVector2D();

        SizeM = std::min(utm_ne.Longitude - utm_nw.Longitude, utm_nw.Latitude - utm_sw.Latitude);

    }
    else if (FGeoReferenceHelper::IsUTM(crs)) {
        int northhemi;
        UTMZone = OSRGetUTMZone(crs, &northhemi);
        bNorthernHemisphere = (northhemi == TRUE);
        Location = UGeoCoordinate((east - west) / 2 + west, (north - south) / 2 + south, EGeoCoordinateType::GCT_UTM, UTMZone, bNorthernHemisphere);
        UTMCoordinates = Location.ToFVector2D();
        WGS84Coordinates = Location.ToWGS84().ToFVector2D();
        SizeM = std::abs(std::min((east - west), (south - north)));
        // UE_LOG(LogTemp, Warning, TEXT("URegionOfInterest: Zone: %d Loc: %f:%f UTM: %f:%f WGS: %f:%f"), UTMZone, Location.Longitude, Location.Latitude, UTMCoordinates.X, UTMCoordinates.Y, WGS84Coordinates.X, WGS84Coordinates.Y);
    } else {
        UE_LOG(LogTemp, Warning, TEXT("URegionOfInterest: Unknown CRS: %s"), crsString);
    }
}

FVector2D URegionOfInterest::GetCorner(EROICorner corner, EGeoCoordinateType coordinatetype)
{
	UGeoCoordinate cornerCoords;
    if (coordinatetype == EGeoCoordinateType::GCT_UNDEFINED) {
        return FVector2D();
    }

	if (corner == EROICorner::NW) {
		cornerCoords = UGeoCoordinate(UTMCoordinates.X - SizeM/2, UTMCoordinates.Y + SizeM/2, EGeoCoordinateType::GCT_UTM, UTMZone, bNorthernHemisphere);
        UE_LOG(LogTemp, Warning, TEXT("URegionOfInterest: NW Corner: %s"), *cornerCoords.ToFVector2D().ToString())
	}
	else if (corner == EROICorner::SW) {
		cornerCoords = UGeoCoordinate(UTMCoordinates.X - SizeM/2, UTMCoordinates.Y - SizeM/2, EGeoCoordinateType::GCT_UTM, UTMZone, bNorthernHemisphere);
        UE_LOG(LogTemp, Warning, TEXT("URegionOfInterest: SW Corner: %s"), *cornerCoords.ToFVector2D().ToString())
	}
	else if (corner == EROICorner::NE) {
		cornerCoords = UGeoCoordinate(UTMCoordinates.X + SizeM/2, UTMCoordinates.Y + SizeM/2, EGeoCoordinateType::GCT_UTM, UTMZone, bNorthernHemisphere);
        UE_LOG(LogTemp, Warning, TEXT("URegionOfInterest: NE Corner: %s"), *cornerCoords.ToFVector2D().ToString())
	}
	else if (corner == EROICorner::SE) {
		cornerCoords = UGeoCoordinate(UTMCoordinates.X + SizeM/2, UTMCoordinates.Y - SizeM/2, EGeoCoordinateType::GCT_UTM, UTMZone, bNorthernHemisphere);
        UE_LOG(LogTemp, Warning, TEXT("URegionOfInterest: SE Corner: %s"), *cornerCoords.ToFVector2D().ToString())
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

		UGeoCoordinate bCoord(borderCoords.X, borderCoords.Y, EGeoCoordinateType::GCT_UTM, UTMZone, bNorthernHemisphere);

		//transform to wsg84
		auto wgsc = bCoord.ToUTM();

		// select x or y
		if (border == EROIBorder::West || border == EROIBorder::East) {
			bordercoord = bCoord.Longitude;
		}
		else {
			bordercoord = bCoord.Latitude;
		}
	}
	return bordercoord;
}

bool URegionOfInterest::Surrounds(UGeoCoordinate &coord)
{
    UGeoCoordinate utm = coord.ToUTM();
    float west = GetBorder(EROIBorder::West, EGeoCoordinateType::GCT_UTM);
    float east = GetBorder(EROIBorder::East, EGeoCoordinateType::GCT_UTM);
    float south = GetBorder(EROIBorder::South, EGeoCoordinateType::GCT_UTM);
    float north = GetBorder(EROIBorder::North, EGeoCoordinateType::GCT_UTM);

    return (utm.Longitude >= west && utm.Longitude <= east
        && utm.Latitude >= south && utm.Latitude <= north);
}

FString URegionOfInterest::ToString()
{
    FString locType;

    if (Location.Type == EGeoCoordinateType::GCT_UNDEFINED){
        locType = TEXT("GCT_Undefined");
    } else if (Location.Type == EGeoCoordinateType::GCT_UTM){
        locType = TEXT("GCT_UTM");
    } else if (Location.Type == EGeoCoordinateType::GCT_WGS84){
        locType = TEXT("GCT_WGS84");
    }
	return TEXT("LocType:") + locType + TEXT(" WGS84:") + WGS84Coordinates.ToString() + TEXT("; UTM:") + UTMCoordinates.ToString()
		+ TEXT(";Size:") + FString::SanitizeFloat(SizeM) + TEXT(";UTMZone:") + FString::FromInt(UTMZone)
		+ TEXT(";NorthHemi:") + (bNorthernHemisphere ? TEXT("true") : TEXT("false"));
}
