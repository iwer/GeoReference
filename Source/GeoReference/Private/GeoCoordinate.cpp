// Copyright (c) Iwer Petersen. All rights reserved.


#include "GeoCoordinate.h"
#include "ROI.h"
#include "GeoReferenceHelper.h"

UGeoCoordinate::UGeoCoordinate()
    : Longitude(0)
    , Latitude(0)
{}

UGeoCoordinate::UGeoCoordinate(double longitude, double latitude, int EPSGNumber)
    : UGeoCoordinate()
{
    // transform input coordinates in crs EPSGNumber to WGS84
    OGRSpatialReference sourceSRS;
    FString wkt = GDALHelpers::WktFromEPSG(EPSGNumber,true);
    auto err = sourceSRS.importFromWkt(TCHAR_TO_ANSI(*wkt));
    if(err != OGRERR_NONE) {
        UE_LOG(LogTemp, Error, TEXT("UGeoCoordinate: Error constructing OGRSpatialReference from EPSG number %d"), EPSGNumber);
    }

    OGRSpatialReference targetSRS;
    targetSRS.SetWellKnownGeogCS("WGS84");

    OGRPoint point(longitude, latitude);

    point.assignSpatialReference(&sourceSRS);
    point.transformTo(&targetSRS);

    Longitude = point.getX();
    Latitude = point.getY();
}

FVector2D UGeoCoordinate::ToFVector2D()
{
    return FVector2D(Longitude, Latitude);
}
FVector2D UGeoCoordinate::ToFVector2DInUTM()
{
    int utmZone = FGeoReferenceHelper::GetUTMZone(Longitude, Latitude);
    int epsgNum = FGeoReferenceHelper::GetEPSGForUTM(utmZone, Latitude>=0);
    return ToFVector2DInEPSG(epsgNum);
}

FVector2D UGeoCoordinate::ToFVector2DInEPSG(int EPSGNumber)
{
    OGRSpatialReference sourceSRS;
    sourceSRS.SetWellKnownGeogCS("WGS84");

    OGRSpatialReference targetSRS;
    FString wkt = GDALHelpers::WktFromEPSG(EPSGNumber, true);
    auto err = targetSRS.importFromWkt(TCHAR_TO_ANSI(*wkt));
    if(err != OGRERR_NONE) {
        UE_LOG(LogTemp, Error, TEXT("UGeoCoordinate: Error constructing OGRSpatialReference from EPSG number %d"), EPSGNumber);
        return FVector2D();
    }
    OGRPoint point(Longitude, Latitude);
    point.assignSpatialReference(&sourceSRS);
    point.transformTo(&targetSRS);

    return FVector2D(point.getX(), point.getY());
}

FVector UGeoCoordinate::ToGameCoordinate(URegionOfInterest &ROI)
{
    // calculate utm coordinate of lon/lat in utm zone of roi origin
    int roiUtmZone = FGeoReferenceHelper::GetUTMZone(ROI.Location.Longitude, ROI.Location.Latitude);
    int epsgNum = FGeoReferenceHelper::GetEPSGForUTM(roiUtmZone, ROI.Location.Latitude >= 0);

    auto diff = Subtract(ROI.Location, epsgNum);

    // calculate relative to roi origin and flip y-axis
    // FVector2D pos = (utm - ROI.Location.ToUTM()) * FVector2D(1, -1);

    // scale m to cm
    // pos *= 100;

    return FVector(diff.X * 100, diff.Y * -100, 0);
}

UGeoCoordinate::DVector2D UGeoCoordinate::Add(UGeoCoordinate &other, int EPSGNumber)
{
    if(EPSGNumber == EPSG_WGS84) {
        return UGeoCoordinate::DVector2D(Longitude + other.Longitude, Latitude + other.Latitude);
    }

    OGRSpatialReference sourceSRS;
    sourceSRS.SetWellKnownGeogCS("WGS84");

    OGRSpatialReference targetSRS;
    FString wkt = GDALHelpers::WktFromEPSG(EPSGNumber, true);
    auto err = targetSRS.importFromWkt(TCHAR_TO_ANSI(*wkt));
    if(err != OGRERR_NONE) {
        UE_LOG(LogTemp, Error, TEXT("UGeoCoordinate: Error constructing OGRSpatialReference from EPSG number %d"), EPSGNumber);
        return UGeoCoordinate::DVector2D();
    }

    OGRPoint thisPoint(Longitude, Latitude);
    OGRPoint otherPoint(other.Longitude, other.Latitude);

    thisPoint.assignSpatialReference(&sourceSRS);
    thisPoint.transformTo(&targetSRS);

    otherPoint.assignSpatialReference(&sourceSRS);
    otherPoint.transformTo(&targetSRS);

    return UGeoCoordinate::DVector2D(thisPoint.getX() + otherPoint.getX(), thisPoint.getY() + otherPoint.getY());
}

UGeoCoordinate::DVector2D UGeoCoordinate::Subtract(UGeoCoordinate &other, int EPSGNumber)
{
    if(EPSGNumber == EPSG_WGS84) {
        return UGeoCoordinate::DVector2D(Longitude - other.Longitude, Latitude - other.Latitude);
    }

    OGRSpatialReference sourceSRS;
    sourceSRS.SetWellKnownGeogCS("WGS84");

    OGRSpatialReference targetSRS;
    FString wkt = GDALHelpers::WktFromEPSG(EPSGNumber,true);
    auto err = targetSRS.importFromWkt(TCHAR_TO_ANSI(*wkt));
    if(err != OGRERR_NONE) {
        UE_LOG(LogTemp, Error, TEXT("UGeoCoordinate: Error constructing OGRSpatialReference from EPSG number %d"), EPSGNumber);
        return UGeoCoordinate::DVector2D();
    }

    OGRPoint thisPoint(Longitude, Latitude);
    OGRPoint otherPoint(other.Longitude, other.Latitude);

    thisPoint.assignSpatialReference(&sourceSRS);
    thisPoint.transformTo(&targetSRS);

    otherPoint.assignSpatialReference(&sourceSRS);
    otherPoint.transformTo(&targetSRS);

    return UGeoCoordinate::DVector2D(thisPoint.getX() - otherPoint.getX(), thisPoint.getY() - otherPoint.getY());
}
