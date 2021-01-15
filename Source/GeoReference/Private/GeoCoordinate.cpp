// Copyright (c) Iwer Petersen. All rights reserved.


#include "GeoCoordinate.h"
#include "ROI.h"
#include "GDALHelpers.h"

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
    int utmZone = GetUTMZone(Longitude, Latitude);
    int epsgNum = GetEPSGForUTM(utmZone, Latitude>=0);
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
    int roiUtmZone = GetUTMZone(ROI.Location.Longitude, ROI.Location.Latitude);
    int epsgNum = GetEPSGForUTM(roiUtmZone, ROI.Location.Latitude >= 0);

    auto diff = Subtract(ROI.Location, epsgNum);

    // calculate relative to roi origin and flip y-axis
    // FVector2D pos = (utm - ROI.Location.ToUTM()) * FVector2D(1, -1);

    // scale m to cm
    // pos *= 100;

    return FVector(diff.X * 100, diff.Y * -100, 0);
}

int UGeoCoordinate::GetUTMZone(double Lon, double Lat)
{
    int utmzone = floor((Lon + 180) / 6) + 1;

    if (Lat >= 56.0 && Lat < 64.0 && Lon >= 3.0 && Lon < 12.0)
        utmzone = 32;

    // Special zones for Svalbard
    if (Lat >= 72.0 && Lat < 84.0) {
        if (Lon >= 0.0 && Lon < 9.0) {
            utmzone = 31;
        }
        else if (Lon >= 9.0 && Lon < 21.0) {
            utmzone = 33;
        }
        else if (Lon >= 21.0 && Lon < 33.0) {
            utmzone = 35;
        }
        else if (Lon >= 33.0 && Lon < 42.0) {
            utmzone = 37;
        }
    }
    return utmzone;
}

char UGeoCoordinate::GetUTMLetter(double Lat)
{
    char LetterDesignator;

    if ((84 >= Lat) && (Lat >= 72)) LetterDesignator = 'X';
    else if ((72 > Lat) && (Lat >= 64)) LetterDesignator = 'W';
    else if ((64 > Lat) && (Lat >= 56)) LetterDesignator = 'V';
    else if ((56 > Lat) && (Lat >= 48)) LetterDesignator = 'U';
    else if ((48 > Lat) && (Lat >= 40)) LetterDesignator = 'T';
    else if ((40 > Lat) && (Lat >= 32)) LetterDesignator = 'S';
    else if ((32 > Lat) && (Lat >= 24)) LetterDesignator = 'R';
    else if ((24 > Lat) && (Lat >= 16)) LetterDesignator = 'Q';
    else if ((16 > Lat) && (Lat >= 8)) LetterDesignator = 'P';
    else if ((8 > Lat) && (Lat >= 0)) LetterDesignator = 'N';
    else if ((0 > Lat) && (Lat >= -8)) LetterDesignator = 'M';
    else if ((-8 > Lat) && (Lat >= -16)) LetterDesignator = 'L';
    else if ((-16 > Lat) && (Lat >= -24)) LetterDesignator = 'K';
    else if ((-24 > Lat) && (Lat >= -32)) LetterDesignator = 'J';
    else if ((-32 > Lat) && (Lat >= -40)) LetterDesignator = 'H';
    else if ((-40 > Lat) && (Lat >= -48)) LetterDesignator = 'G';
    else if ((-48 > Lat) && (Lat >= -56)) LetterDesignator = 'F';
    else if ((-56 > Lat) && (Lat >= -64)) LetterDesignator = 'E';
    else if ((-64 > Lat) && (Lat >= -72)) LetterDesignator = 'D';
    else if ((-72 > Lat) && (Lat >= -80)) LetterDesignator = 'C';
    else LetterDesignator = 'Z'; //This is here as an error flag to show that the Lat is outside the UTM limits

    return LetterDesignator;
}

int UGeoCoordinate::GetEPSGForUTM(int utmZone, bool bNorthernHemi)
{
    int epsgCode = -1;
    if(utmZone != -1) {
        epsgCode = 32600;
        epsgCode += utmZone;
        if(!bNorthernHemi)
            epsgCode += 100;
    }
    return epsgCode;
}

// UGeoCoordinate UGeoCoordinate::operator+(const UGeoCoordinate & other)
// {
//     if(Type == other.Type) {
//         if(Type == EGeoCoordinateType::GCT_UTM && (UTMZone != other.UTMZone || bNorthernHemisphere != other.bNorthernHemisphere)){
//             UE_LOG(LogTemp, Error, TEXT("UGeoCoordinate: Operator+ needs UTMCoordinates in the same zone."))
//             return UGeoCoordinate();
//         }
//         return UGeoCoordinate(Longitude + other.Longitude,
//                             Latitude + other.Latitude,
//                             Type, UTMZone, bNorthernHemisphere);
//     }
//     UE_LOG(LogTemp, Error, TEXT("UGeoCoordinate: Operator+ needs arguments of same type."))
//     return UGeoCoordinate();
// }
//
// UGeoCoordinate UGeoCoordinate::operator-(const UGeoCoordinate & other)
// {
//     if(Type == other.Type) {
//         if(Type == EGeoCoordinateType::GCT_UTM && (UTMZone != other.UTMZone || bNorthernHemisphere != other.bNorthernHemisphere)){
//             UE_LOG(LogTemp, Error, TEXT("UGeoCoordinate: Operator- needs UTMCoordinates in the same zone.(this:%d, %d; Other: %d, %d)"),UTMZone, bNorthernHemisphere, other.UTMZone, other.bNorthernHemisphere)
//             return UGeoCoordinate();
//         }
//         return UGeoCoordinate(Longitude - other.Longitude,
//                             Latitude - other.Latitude,
//                             Type, UTMZone, bNorthernHemisphere);
//     }
//     UE_LOG(LogTemp, Error, TEXT("UGeoCoordinate: Operator- needs arguments of same type."))
//     return UGeoCoordinate();
// }
//
// FVector2D UGeoCoordinate::operator*(const FVector2D & other)
// {
//     return FVector2D(Longitude * other.X, Latitude * other.Y);
// }

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
