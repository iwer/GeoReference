// Copyright (c) Iwer Petersen. All rights reserved.


#include "GeoReferenceActor.h"


// Sets default values
AGeoReferenceActor::AGeoReferenceActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	ROI = CreateDefaultSubobject<URegionOfInterest>(FName(TEXT("RegionOfInterest")));
}

// Called when the game starts or when spawned
void AGeoReferenceActor::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AGeoReferenceActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGeoReferenceActor::OnConstruction(const FTransform & Transform)
{
    // UE_LOG(LogTemp, Warning, TEXT("AGeoReferenceActor: OnConstruction"))
    ROI->Init(FVector2D(Longitude,Latitude), SizeM);
    // UE_LOG(LogTemp, Warning, TEXT("AGeoReferenceActor: ROI: %s"), *ROI->ToString())

    if(bShowBounds) {
        FVector2D nw = ROI->GetCorner(EROICorner::NW, EGeoCoordinateType::GCT_WGS84);
        FVector2D sw = ROI->GetCorner(EROICorner::SW, EGeoCoordinateType::GCT_WGS84);
        FVector2D ne = ROI->GetCorner(EROICorner::NE, EGeoCoordinateType::GCT_WGS84);
        FVector2D se = ROI->GetCorner(EROICorner::SE, EGeoCoordinateType::GCT_WGS84);

        FVector nwg = ToGameCoordinate(FVector(nw, 0));
        FVector swg = ToGameCoordinate(FVector(sw, 0));
        FVector neg = ToGameCoordinate(FVector(ne, 0));
        FVector seg = ToGameCoordinate(FVector(se, 0));

        UE_LOG(LogTemp, Warning, TEXT("AGeoReferenceActor: Bounds %s; %s; %s; %s"),*nwg.ToString(),*swg.ToString(),*neg.ToString(),*seg.ToString());

        DrawDebugLine(GetWorld(), nwg, swg, FColor::Green, false, 20.f, false, 4.f);
        DrawDebugLine(GetWorld(), nwg, neg, FColor::Green, false, 20.f, false, 4.f);
        DrawDebugLine(GetWorld(), swg, seg, FColor::Green, false, 20.f, false, 4.f);
        DrawDebugLine(GetWorld(), neg, seg, FColor::Green, false, 20.f, false, 4.f);

        DrawDebugSphere(GetWorld(), nwg, 200.0, 8, FColor::Red, true, 20.0, false, 5.);
        DrawDebugSphere(GetWorld(), swg, 200.0, 8, FColor::Red, true, 20.0, false, 5.);
        DrawDebugSphere(GetWorld(), neg, 200.0, 8, FColor::Red, true, 20.0, false, 5.);
        DrawDebugSphere(GetWorld(), seg, 200.0, 8, FColor::Red, true, 20.0, false, 5.);


    }
}

FVector AGeoReferenceActor::ToGameCoordinate(FVector geocoordinate)
{
    if(!ROI) {
        UE_LOG(LogTemp, Warning, TEXT("AGeoReferenceActor: No ROI defined!"))
        return FVector::ZeroVector;
    }

    // see if there is a landscape
    ALandscape * landscape = nullptr;
    for (TObjectIterator<ALandscape> Itr; Itr; ++Itr)
    {
        if(Itr->IsA(ALandscape::StaticClass())){
            landscape = *Itr;
            break;
        } else {
            continue;
        }
    }

    UGeoCoordinate geocoord(geocoordinate.X, geocoordinate.Y, EGeoCoordinateType::GCT_WGS84);
    FVector gamecoord = geocoord.ToGameCoordinate(*ROI);

    if(landscape){
        FVector origin, boxExtends;
        landscape->GetActorBounds(true, origin, boxExtends, false);

        // UE_LOG(LogTemp, Warning, TEXT("AGeoReferenceActor: Landscape origin: %s"), *origin.ToString())
        gamecoord += origin;
    }
    // UE_LOG(LogTemp,Warning,TEXT("AGeoReferenceActor: 2GC ROI: %s"), *ROI->ToString())
    gamecoord.Z = geocoordinate.Z;
	return gamecoord;
}

FVector AGeoReferenceActor::ToGeoCoordinate(FVector gamecoordinate)
{
    if(!ROI) {
        UE_LOG(LogTemp, Warning, TEXT("AGeoReferenceActor: No ROI defined!"))
        return FVector::ZeroVector;
    }

    UGeoCoordinate coord = CalculateGeoLocation(gamecoordinate);

    // make WGS48 FVector and return
    return FVector(coord.ToWGS84().ToFVector2D(), 0);
}

UGeoCoordinate AGeoReferenceActor::CalculateGeoLocation(FVector gamecoordinate) {
    // see if there is a landscape
    ALandscape * landscape = nullptr;
    for (TObjectIterator<ALandscape> Itr; Itr; ++Itr)
    {
        if(Itr->IsA(ALandscape::StaticClass())){
            landscape = *Itr;
            break;
        } else {
            continue;
        }
    }
    FVector geocoord = gamecoordinate;

    // subtract landscape center from coordinate
    if(landscape){
        FVector origin, boxExtends;
        landscape->GetActorBounds(true, origin, boxExtends, false);

        // UE_LOG(LogTemp, Warning, TEXT("AGeoReferenceActor: Landscape origin: %s"), *origin.ToString())
        geocoord -= origin;
    }

    // to meters with reverse y direction
    geocoord *= FVector(.01, -.01, .01);

    // add UTM offset of ROI center
    geocoord += FVector(ROI->Location.ToUTM().ToFVector2D(), 0);

    // Make UTM geocoord
    return UGeoCoordinate(geocoord.X, geocoord.Y, EGeoCoordinateType::GCT_UTM, ROI->UTMZone, ROI->bNorthernHemisphere);
}

bool AGeoReferenceActor::IsGameCoordInsideROI(FVector gamecoord)
{
    if(!ROI)
        return false;

    auto coord = CalculateGeoLocation(gamecoord);
    return ROI->Surrounds(coord);
}

bool AGeoReferenceActor::IsGeoCoordInsideROI(FVector geocoord)
{
    if(!ROI)
        return false;

    UGeoCoordinate coord(geocoord.X, geocoord.Y, EGeoCoordinateType::GCT_WGS84);
    return ROI->Surrounds(coord);
}
