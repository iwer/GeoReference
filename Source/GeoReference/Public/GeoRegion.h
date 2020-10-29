// Copyright (c) Iwer Petersen. All rights reserved.

#pragma once

#include "Engine.h"
#include "GeoReference.h"
#include "GDALHelpers.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GeoRegion.generated.h"

USTRUCT(BlueprintType)
struct FGeoRegion
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GeoReference")
    float Longitude;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GeoReference")
    float Latitude;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GeoReference")
    float SizeM;

    UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category="GeoReference")
    float West;

    UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category="GeoReference")
    float East;

    UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category="GeoReference")
    float South;

    UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category="GeoReference")
    float North;

    UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category="GeoReference")
    int UTMZone;

    UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category="GeoReference")
    float UTMWest;

    UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category="GeoReference")
    float UTMEast;

    UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category="GeoReference")
    float UTMSouth;

    UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category="GeoReference")
    float UTMNorth;

    UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category="GeoReference")
    bool NorthernHemisphere;

    void SetFromLocationAndSize(float longitude,float latitude,  float sizeM)
    {
        Longitude = longitude;
        Latitude = latitude;
        SizeM = sizeM;

        FVector2D center_point_utm = FGeoReference::TransformWGSToUTM(longitude, latitude);

        UTMZone = FGeoReference::UTMZone(longitude, latitude);
        NorthernHemisphere = latitude >= 0;

        UTMWest = center_point_utm.X - sizeM / 2;
        UTMEast = center_point_utm.X + sizeM / 2;
        UTMNorth = center_point_utm.Y + sizeM / 2;
        UTMSouth = center_point_utm.Y - sizeM / 2;

        // in UTM North and East are positive
        FVector2D n_point_wsg = FGeoReference::TransformUTMToWGS(center_point_utm.X, UTMNorth, UTMZone, latitude >= 0);
        FVector2D s_point_wsg = FGeoReference::TransformUTMToWGS(center_point_utm.X, UTMSouth, UTMZone, latitude >= 0);
        FVector2D w_point_wsg = FGeoReference::TransformUTMToWGS(UTMWest, center_point_utm.Y, UTMZone, latitude >= 0);
        FVector2D e_point_wsg = FGeoReference::TransformUTMToWGS(UTMEast, center_point_utm.Y, UTMZone, latitude >= 0);

        West = w_point_wsg.X;
        East = e_point_wsg.X;
        South = s_point_wsg.Y;
        North = n_point_wsg.Y;
    }

    void SetFromGDALDataset(GDALDatasetRef &dataset)
    {
        GeoTransformRef georef = GDALHelpers::GetGeoTransform(dataset);
        FVector2D srcsize = FVector2D(dataset->GetRasterXSize(), dataset->GetRasterYSize());
        double north = georef[3];
        double west = georef[0];
        double south = georef[3] + srcsize.X * georef[4] + srcsize.Y * georef[5];
        double east  = georef[0] + srcsize.X * georef[1] + srcsize.Y * georef[2];

        auto crs_s = dataset->GetProjectionRef();
        auto crs = OSRNewSpatialReference(crs_s);
        if(FGeoReference::IsWGS84(crs)){
            Longitude = (east - west)  / 2 + west;
            Latitude  = (north - south) / 2 + south;
            West = west;
            East = east;
            North = north;
            South = south;

        } else if(FGeoReference::IsUTM(crs)) {
            // transform to wgs84
            int northhemi;
            UTMZone = OSRGetUTMZone(crs, &northhemi);
            NorthernHemisphere = (northhemi == TRUE);
            auto center = FGeoReference::TransformUTMToWGS((east - west)  / 2 + west, (north - south) / 2 + south, UTMZone, NorthernHemisphere);
            auto wgs_nw = FGeoReference::TransformUTMToWGS(west, north, UTMZone, NorthernHemisphere);
            auto wgs_se = FGeoReference::TransformUTMToWGS(east, south, UTMZone, NorthernHemisphere);
            Longitude = center.X;
            Latitude  = center.Y;
            West  = wgs_nw.X;
            East  = wgs_se.X;
            North = wgs_nw.Y;
            South = wgs_se.Y;
            UTMNorth = north;
            UTMEast = east;
            UTMWest = west;
            UTMSouth = south;
        } else {
            Longitude=0;
            Latitude=0;
            West=0;
            East=0;
            North=0;
            South=0;
            UTMZone=-1;
        }
    }

    FGeoRegion()
        : Longitude(0)
        , Latitude(0)
        , SizeM(0)
        , West(0)
        , East(0)
        , South(0)
        , North(0)
        , UTMZone(-1)
        , UTMWest(0)
        , UTMEast(0)
        , UTMSouth(0)
        , UTMNorth(0)
        , NorthernHemisphere(true)
    {}

    FGeoRegion(float longitude,float latitude,  float sizeM)
    {
        SetFromLocationAndSize(longitude,latitude,sizeM);
    }

    FGeoRegion(GDALDatasetRef gdaldata)
    {
        SetFromGDALDataset(gdaldata);
    }

};


UCLASS()
class GEOREFERENCE_API UGeoRegionBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="GeoRegion|Methods")
    static void SetFromLocationAndSize(UPARAM(ref) FGeoRegion& GeoRegion, float longitude,float latitude,  float sizeM)
    {
        GeoRegion.Longitude = longitude;
        GeoRegion.Latitude = latitude;
        GeoRegion.SizeM = sizeM;

        FVector2D center_point_utm = FGeoReference::TransformWGSToUTM(longitude, latitude);

        GeoRegion.UTMZone = FGeoReference::UTMZone(longitude, latitude);
        GeoRegion.NorthernHemisphere = latitude >= 0;
        // in UTM North and East are positive
        GeoRegion.UTMWest = center_point_utm.X - sizeM / 2;
        GeoRegion.UTMEast = center_point_utm.X + sizeM / 2;
        GeoRegion.UTMNorth = center_point_utm.Y + sizeM / 2;
        GeoRegion.UTMSouth = center_point_utm.Y - sizeM / 2;

        // in UTM North and East are positive
        FVector2D n_point_wsg = FGeoReference::TransformUTMToWGS(center_point_utm.X, GeoRegion.UTMNorth, GeoRegion.UTMZone, GeoRegion.NorthernHemisphere);
        FVector2D s_point_wsg = FGeoReference::TransformUTMToWGS(center_point_utm.X, GeoRegion.UTMSouth, GeoRegion.UTMZone, GeoRegion.NorthernHemisphere);
        FVector2D w_point_wsg = FGeoReference::TransformUTMToWGS(GeoRegion.UTMWest, center_point_utm.Y, GeoRegion.UTMZone, GeoRegion.NorthernHemisphere);
        FVector2D e_point_wsg = FGeoReference::TransformUTMToWGS(GeoRegion.UTMEast, center_point_utm.Y, GeoRegion.UTMZone, GeoRegion.NorthernHemisphere);

        GeoRegion.West = w_point_wsg.X;
        GeoRegion.East = e_point_wsg.X;
        GeoRegion.South = s_point_wsg.Y;
        GeoRegion.North = n_point_wsg.Y;
    }

};
