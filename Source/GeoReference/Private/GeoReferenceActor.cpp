// Copyright (c) Iwer Petersen. All rights reserved.


#include "GeoReferenceActor.h"


// Sets default values
AGeoReferenceActor::AGeoReferenceActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

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


FVector AGeoReferenceActor::ToGameCoordinate(FVector geocoordinate)
{
    return FGeoReference::ToGameCoordinate(geocoordinate.X, geocoordinate.Y, Region.West, Region.North, Region.South);
}

FVector AGeoReferenceActor::ToGeoCoordinate(FVector gamecoordinate)
{
    return FVector();
}
