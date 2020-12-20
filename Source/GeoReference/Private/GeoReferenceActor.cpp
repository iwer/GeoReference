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

    if(ShowBounds) {
        FVector2D nw = ROI->GetCorner(EROICorner::NW, EGeoCoordinateType::GCT_WGS84);
        FVector2D sw = ROI->GetCorner(EROICorner::SW, EGeoCoordinateType::GCT_WGS84);
        FVector2D ne = ROI->GetCorner(EROICorner::NE, EGeoCoordinateType::GCT_WGS84);
        FVector2D se = ROI->GetCorner(EROICorner::SE, EGeoCoordinateType::GCT_WGS84);

        FVector nwg = ToGameCoordinate(FVector(nw, 0));
        FVector swg = ToGameCoordinate(FVector(sw, 0));
        FVector neg = ToGameCoordinate(FVector(ne, 0));
        FVector seg = ToGameCoordinate(FVector(se, 0));

        DrawDebugLine(GetWorld(), nwg, swg, FColor::Green, false, 5.f, false, 4.f);
        DrawDebugLine(GetWorld(), nwg, neg, FColor::Green, false, 5.f, false, 4.f);
        DrawDebugLine(GetWorld(), swg, seg, FColor::Green, false, 5.f, false, 4.f);
        DrawDebugLine(GetWorld(), neg, seg, FColor::Green, false, 6.f, false, 4.f);
    }
}

FVector AGeoReferenceActor::ToGameCoordinate(FVector geocoordinate)
{
    if(!ROI) {
        UE_LOG(LogTemp, Warning, TEXT("AGeoReferenceActor: No ROI defined!"))
        return FVector::ZeroVector;
    }
    // UE_LOG(LogTemp,Warning,TEXT("AGeoReferenceActor: 2GC ROI: %s"), *ROI->ToString())
	return FGeoReference::ToGameCoordinate(geocoordinate.X, geocoordinate.Y, *ROI);
}

FVector AGeoReferenceActor::ToGeoCoordinate(FVector gamecoordinate)
{
    return FVector();
}
