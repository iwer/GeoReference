// Copyright (c) Iwer Petersen. All rights reserved.


#include "ROI.h"
#include "GeoReferenceHelper.h"


URegionOfInterest::URegionOfInterest()
{
    SizeM = 0.0;
    UTMZone = -1;
}

URegionOfInterest::URegionOfInterest(FVector2D geocoordinates, float size)
    : Location(geocoordinates.X, geocoordinates.Y, UGeoCoordinate::EPSG_WGS84)
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
        double approxLon = west + (east - west) / 2;
        double approxLat = south + (north - south) / 2;
        int epsgNum = FGeoReferenceHelper::GetEPSGForUTM(FGeoReferenceHelper::GetUTMZone(approxLon, approxLat), approxLat >= 0);

        auto utm_nw = UGeoCoordinate(west, north, UGeoCoordinate::EPSG_WGS84).ToFVector2DInEPSG(epsgNum);
        auto utm_ne = UGeoCoordinate(east, north, UGeoCoordinate::EPSG_WGS84).ToFVector2DInEPSG(epsgNum);
        auto utm_sw = UGeoCoordinate(west, south, UGeoCoordinate::EPSG_WGS84).ToFVector2DInEPSG(epsgNum);

        OutWidth = utm_ne.X - utm_nw.X;
        OutHeight = utm_nw.Y - utm_sw.Y;
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
    Location = UGeoCoordinate(geocoordinates.X, geocoordinates.Y, UGeoCoordinate::EPSG_WGS84);
    SizeM = size;
    WGS84Coordinates = geocoordinates;
    UTMCoordinates = Location.ToFVector2DInUTM();

    UTMZone = FGeoReferenceHelper::GetUTMZone(geocoordinates.X, geocoordinates.Y);
    bNorthernHemisphere = geocoordinates.Y >= 0;

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
    char * unitName;
    double toMeter = OSRGetLinearUnits(crs, &unitName);

    if (FGeoReferenceHelper::IsWGS84(crs)) {
        // rough (wgs) center for utmzone calculation
        UTMZone = FGeoReferenceHelper::GetUTMZone(west + (east - west), south + (north - south));
        bNorthernHemisphere = (south + (north - south) >= 0);
        int epsgNum = FGeoReferenceHelper::GetEPSGForUTM(UTMZone, bNorthernHemisphere);


        auto utm_nw = UGeoCoordinate(west, north, UGeoCoordinate::EPSG_WGS84).ToFVector2DInEPSG(epsgNum);
        auto utm_sw = UGeoCoordinate(west, south, UGeoCoordinate::EPSG_WGS84).ToFVector2DInEPSG(epsgNum);
        auto utm_ne = UGeoCoordinate(east, north, UGeoCoordinate::EPSG_WGS84).ToFVector2DInEPSG(epsgNum);
        auto utm_se = UGeoCoordinate(east, south, UGeoCoordinate::EPSG_WGS84).ToFVector2DInEPSG(epsgNum);

        UGeoCoordinate center(utm_ne.X - utm_nw.X, utm_nw.Y - utm_sw.Y, epsgNum);

        Location = center;
        WGS84Coordinates = Location.ToFVector2DInEPSG(UGeoCoordinate::EPSG_WGS84);
        UTMCoordinates = Location.ToFVector2DInEPSG(epsgNum);

        SizeM = std::min(utm_ne.X - utm_nw.X, utm_nw.Y - utm_sw.Y);

    }
    // else if (FGeoReferenceHelper::IsUTM(crs)) {
    else if (OSRIsProjected(crs) && strncmp(unitName, "metre", 6) == 0) {
        int northhemi;
        UTMZone = OSRGetUTMZone(crs, &northhemi);
        bNorthernHemisphere = (northhemi == TRUE);
        Location = UGeoCoordinate((east - west) / 2 + west, (north - south) / 2 + south, FGeoReferenceHelper::GetEPSGForUTM(UTMZone, bNorthernHemisphere));

        UTMCoordinates = FVector2D((east - west) / 2 + west, (north - south) / 2 + south);
        WGS84Coordinates = Location.ToFVector2DInEPSG(UGeoCoordinate::EPSG_WGS84);
        SizeM = std::abs(std::min((east - west), (south - north)));
        // UE_LOG(LogTemp, Warning, TEXT("URegionOfInterest: Zone: %d Loc: %f:%f UTM: %f:%f WGS: %f:%f"), UTMZone, Location.Longitude, Location.Latitude, UTMCoordinates.X, UTMCoordinates.Y, WGS84Coordinates.X, WGS84Coordinates.Y);
    } else {
        UE_LOG(LogTemp, Warning, TEXT("URegionOfInterest: Unknown CRS: %s"), ANSI_TO_TCHAR(crsString));
    }
}

FVector2D URegionOfInterest::GetCorner(EROICorner corner, int EPSGNumber)
{
    FVector2D cornerCoords;

    if (corner == EROICorner::NW) {
        cornerCoords = FVector2D(UTMCoordinates.X - SizeM/2, UTMCoordinates.Y + SizeM/2);
    }
    else if (corner == EROICorner::SW) {
        cornerCoords = FVector2D(UTMCoordinates.X - SizeM/2, UTMCoordinates.Y - SizeM/2);
    }
    else if (corner == EROICorner::NE) {
        cornerCoords = FVector2D(UTMCoordinates.X + SizeM/2, UTMCoordinates.Y + SizeM/2);
    }
    else if (corner == EROICorner::SE) {
        cornerCoords = FVector2D(UTMCoordinates.X + SizeM/2, UTMCoordinates.Y - SizeM/2);
    }

    return UGeoCoordinate(cornerCoords.X, cornerCoords.Y, FGeoReferenceHelper::GetEPSGForUTM(UTMZone, bNorthernHemisphere)).ToFVector2DInEPSG(EPSGNumber);
}

float URegionOfInterest::GetBorder(EROIBorder border, int EPSGNumber)
{
    float UTMWest = UTMCoordinates.X - SizeM / 2;
    float UTMEast = UTMCoordinates.X + SizeM / 2;
    float UTMNorth = UTMCoordinates.Y + SizeM / 2;
    float UTMSouth = UTMCoordinates.Y - SizeM / 2;


    float bordercoord = 0.0;

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

    UGeoCoordinate bCoord(borderCoords.X, borderCoords.Y, FGeoReferenceHelper::GetEPSGForUTM(UTMZone, bNorthernHemisphere));

    // transform to target CRS
    auto epsgCoord = bCoord.ToFVector2DInEPSG(EPSGNumber);

    // select x or y
    if (border == EROIBorder::West || border == EROIBorder::East) {
        bordercoord = epsgCoord.X;
    }
    else {
        bordercoord = epsgCoord.Y;
    }

    return bordercoord;
}

bool URegionOfInterest::Surrounds(UGeoCoordinate &coord)
{
    FVector2D utm = coord.ToFVector2DInUTM();
    float west = GetBorder(EROIBorder::West, FGeoReferenceHelper::GetEPSGForUTM(UTMZone, bNorthernHemisphere));
    float east = GetBorder(EROIBorder::East, FGeoReferenceHelper::GetEPSGForUTM(UTMZone, bNorthernHemisphere));
    float south = GetBorder(EROIBorder::South, FGeoReferenceHelper::GetEPSGForUTM(UTMZone, bNorthernHemisphere));
    float north = GetBorder(EROIBorder::North, FGeoReferenceHelper::GetEPSGForUTM(UTMZone, bNorthernHemisphere));

    return (utm.X >= west && utm.X <= east
        && utm.Y >= south && utm.Y <= north);
}

FString URegionOfInterest::ToString()
{

    return TEXT(" WGS84:") + WGS84Coordinates.ToString() + TEXT("; UTM:") + UTMCoordinates.ToString()
        + TEXT(";Size:") + FString::SanitizeFloat(SizeM) + TEXT(";UTMZone:") + FString::FromInt(UTMZone)
        + TEXT(";NorthHemi:") + (bNorthernHemisphere ? TEXT("true") : TEXT("false"));
}
