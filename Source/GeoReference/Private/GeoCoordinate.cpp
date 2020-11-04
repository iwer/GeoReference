// Fill out your copyright notice in the Description page of Project Settings.


#include "GeoCoordinate.h"

UGeoCoordinate::UGeoCoordinate()
	: UGeoCoordinate(0,0,EGeoCoordinateType::GCT_UNDEFINED)
{}

UGeoCoordinate::UGeoCoordinate(double longitude, double latitude, EGeoCoordinateType type)
	: Longitude(longitude)
	, Latitude(latitude)
	, Type(type)
{}

FVector2D UGeoCoordinate::ToFVector2D() 
{
	return FVector2D(Longitude, Latitude);
}

UGeoCoordinate UGeoCoordinate::ToWSG84()
{
	if (Type == EGeoCoordinateType::GCT_UNDEFINED) {
		UE_LOG(LogTemp, Error, TEXT("UGeoCoordinate: Cannot convert undefined coordinate type."));
		return UGeoCoordinate();
	}
	if (Type == EGeoCoordinateType::GCT_WSG84) {
		UE_LOG(LogTemp, Warning, TEXT("UGeoCoordinate: Coordinate is already WSG84, returning copy."));
		return UGeoCoordinate(Longitude, Latitude, Type);
	}

	return UGeoCoordinate();
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

	return UGeoCoordinate();
}