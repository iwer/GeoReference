// Copyright (c) Iwer Petersen. All rights reserved.


#include "GeoCoordinate.h"
#include "ROI.h"
#include "GDALHelpers.h"

UGeoCoordinate::UGeoCoordinate()
	: UGeoCoordinate(0,0,EGeoCoordinateType::GCT_UNDEFINED, -1, true)
{}

UGeoCoordinate::UGeoCoordinate(double longitude, double latitude, EGeoCoordinateType type, int utmZone, bool bNorthernHemi)
	: Longitude(longitude)
	, Latitude(latitude)
	, Type(type)
    , UTMZone(utmZone)
    , bNorthernHemisphere(bNorthernHemi)
{
    if(type==EGeoCoordinateType::GCT_UTM && utmZone == -1) {
        UE_LOG(LogTemp,Error,TEXT("UGeoCoordinate: UTM zone undefined for (%f, %f)"), Latitude, Longitude);
    }
}

FVector2D UGeoCoordinate::ToFVector2D()
{
	return FVector2D(Longitude, Latitude);
}

UGeoCoordinate UGeoCoordinate::ToWGS84()
{
	if (Type == EGeoCoordinateType::GCT_UNDEFINED) {
		UE_LOG(LogTemp, Error, TEXT("UGeoCoordinate: Cannot convert undefined coordinate type."));
		return UGeoCoordinate();
	}
	if (Type == EGeoCoordinateType::GCT_WGS84) {
		// UE_LOG(LogTemp, Warning, TEXT("UGeoCoordinate: Coordinate is already WSG84, returning copy."));
		return UGeoCoordinate(Longitude, Latitude, EGeoCoordinateType::GCT_WGS84);
	}

	return TransformToWGS84();
}

UGeoCoordinate UGeoCoordinate::ToUTM(int utmZone, bool bNorth)
{
    int zone = UTMZone;
    bool nrth = bNorthernHemisphere;
    if(utmZone != -1){
        zone = utmZone;
        nrth = bNorth;
    }

	if (Type == EGeoCoordinateType::GCT_UNDEFINED) {
		UE_LOG(LogTemp, Error, TEXT("UGeoCoordinate: Cannot convert undefined coordinate type."));
		return UGeoCoordinate();
	}
	if (Type == EGeoCoordinateType::GCT_UTM) {
		// UE_LOG(LogTemp, Warning, TEXT("UGeoCoordinate: Coordinate is already UTM, returning copy."));

		return UGeoCoordinate(Longitude, Latitude, EGeoCoordinateType::GCT_UTM, UTMZone, bNorthernHemisphere);
	}

    // UE_LOG(LogTemp,Warning,TEXT("UGeoCoordinate: 2U result: %d %d"), utmZone, bNorth);
	return TransformToUTM(zone, nrth);
}

FVector UGeoCoordinate::ToGameCoordinate(URegionOfInterest &ROI)
{
    // calculate utm coordinate of lon/lat in utm zone of roi origin
    auto utm = ToUTM(ROI.UTMZone, ROI.bNorthernHemisphere);

    // calculate relative to roi origin and flip y-axis
    FVector2D pos = (utm - ROI.Location.ToUTM()) * FVector2D(1, -1);

    // scale m to cm
    pos *= 100;

    return FVector(pos.X, pos.Y, 0);
}

UGeoCoordinate UGeoCoordinate::TransformToWGS84(){
    OGRSpatialReference sourceSRS;
    sourceSRS.SetUTM(UTMZone, bNorthernHemisphere);

    OGRSpatialReference targetSRS;
    targetSRS.SetWellKnownGeogCS("WGS84");

    OGRPoint point(Longitude, Latitude);
    point.assignSpatialReference(&sourceSRS);
    point.transformTo(&targetSRS);
    return UGeoCoordinate(point.getX(), point.getY(), EGeoCoordinateType::GCT_WGS84, -1, true);
}

UGeoCoordinate UGeoCoordinate::TransformToUTM(int TargetUTMZone, bool bTargetNorthernHemi){
    OGRSpatialReference sourceSRS;
    sourceSRS.SetWellKnownGeogCS("WGS84");

    OGRSpatialReference targetSRS;

    int utmzone = UGeoCoordinate::GetUTMZone(Longitude, Latitude);
    bool north = Latitude >= 0;
    if(TargetUTMZone != -1){
        utmzone = TargetUTMZone;
        north = bTargetNorthernHemi;
    }

    targetSRS.SetUTM(utmzone, north);

    OGRPoint point(Longitude, Latitude);
    // if latitude is bigger than 84 or smaller than -80 there is no utm zone
    if (UGeoCoordinate::GetUTMLetter(Latitude) == 'Z') {
        UE_LOG(LogTemp, Error, TEXT("UGeoCoordinate: latitude %f is outside defined UTM Zones!"), Latitude);
        return UGeoCoordinate(0,0,EGeoCoordinateType::GCT_UTM, -1, Latitude >= 0);
    }

    point.assignSpatialReference(&sourceSRS);
    point.transformTo(&targetSRS);
    // UE_LOG(LogTemp,Warning,TEXT("FGeoReferenceHelper: W2U result: %f, %f Zone: %d %d (called with %d %d)"),point.getX(), point.getY(), utmzone, north, utmZone, northernHemi);
    return UGeoCoordinate(point.getX(), point.getY(), EGeoCoordinateType::GCT_UTM, utmzone, north);
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

UGeoCoordinate UGeoCoordinate::operator+(const UGeoCoordinate & other)
{
    if(Type == other.Type) {
        if(Type == EGeoCoordinateType::GCT_UTM && (UTMZone != other.UTMZone || bNorthernHemisphere != other.bNorthernHemisphere)){
            UE_LOG(LogTemp, Error, TEXT("UGeoCoordinate: Operator+ needs UTMCoordinates in the same zone."))
            return UGeoCoordinate();
        }
        return UGeoCoordinate(Longitude + other.Longitude,
                            Latitude + other.Latitude,
                            Type, UTMZone, bNorthernHemisphere);
    }
    UE_LOG(LogTemp, Error, TEXT("UGeoCoordinate: Operator+ needs arguments of same type."))
    return UGeoCoordinate();
}

UGeoCoordinate UGeoCoordinate::operator-(const UGeoCoordinate & other)
{
    if(Type == other.Type) {
        if(Type == EGeoCoordinateType::GCT_UTM && (UTMZone != other.UTMZone || bNorthernHemisphere != other.bNorthernHemisphere)){
            UE_LOG(LogTemp, Error, TEXT("UGeoCoordinate: Operator- needs UTMCoordinates in the same zone.(this:%d, %d; Other: %d, %d)"),UTMZone, bNorthernHemisphere, other.UTMZone, other.bNorthernHemisphere)
            return UGeoCoordinate();
        }
        return UGeoCoordinate(Longitude - other.Longitude,
                            Latitude - other.Latitude,
                            Type, UTMZone, bNorthernHemisphere);
    }
    UE_LOG(LogTemp, Error, TEXT("UGeoCoordinate: Operator- needs arguments of same type."))
    return UGeoCoordinate();
}

FVector2D UGeoCoordinate::operator*(const FVector2D & other)
{
    return FVector2D(Longitude * other.X, Latitude * other.Y);
}
