// Fill out your copyright notice in the Description page of Project Settings.


#include "GeoCoordinate.h"
#include "GeoReference.h"

UGeoCoordinate::UGeoCoordinate()
	: UGeoCoordinate(0,0,EGeoCoordinateType::GCT_UNDEFINED, -1, true)
{}

UGeoCoordinate::UGeoCoordinate(double longitude, double latitude, EGeoCoordinateType type, int utmZone, bool northernHemi)
	: Longitude(longitude)
	, Latitude(latitude)
	, Type(type)
    , UTMZone(utmZone)
    , NorthernHemisphere(northernHemi)
{
    if(type==EGeoCoordinateType::GCT_UTM && utmZone == -1) {
        UE_LOG(LogTemp,Error,TEXT("UGeoCoordinate: UTM zone undefined"));
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
		UE_LOG(LogTemp, Warning, TEXT("UGeoCoordinate: Coordinate is already WSG84, returning copy."));
		return UGeoCoordinate(Longitude, Latitude, Type);
	}

	return FGeoReference::TransformUTMToWGS(Longitude, Latitude, UTMZone, NorthernHemisphere);
}

UGeoCoordinate UGeoCoordinate::ToUTM()
{
	if (Type == EGeoCoordinateType::GCT_UNDEFINED) {
		UE_LOG(LogTemp, Error, TEXT("UGeoCoordinate: Cannot convert undefined coordinate type."));
		return UGeoCoordinate();
	}
	if (Type == EGeoCoordinateType::GCT_UTM) {
		UE_LOG(LogTemp, Warning, TEXT("UGeoCoordinate: Coordinate is already UTM, returning copy."));

		return UGeoCoordinate(Longitude, Latitude, Type);
	}

	return FGeoReference::TransformWGSToUTM(Longitude, Latitude);
}

UGeoCoordinate UGeoCoordinate::operator+(const UGeoCoordinate & other)
{
    if(Type == other.Type) {
        return UGeoCoordinate(Longitude + other.Longitude,
                            Latitude + other.Latitude,
                            Type);
    }
    UE_LOG(LogTemp, Error, TEXT("UGeoCoordinate: Operator+ needs arguments of same type."))
    return UGeoCoordinate();
}

UGeoCoordinate UGeoCoordinate::operator-(const UGeoCoordinate & other)
{
    if(Type == other.Type) {
        return UGeoCoordinate(Longitude - other.Longitude,
                            Latitude - other.Latitude,
                            Type);
    }
    UE_LOG(LogTemp, Error, TEXT("UGeoCoordinate: Operator- needs arguments of same type."))
    return UGeoCoordinate();
}

FVector2D UGeoCoordinate::operator*(const FVector2D & other)
{
    return FVector2D(Longitude * other.X, Latitude * other.Y);
}
