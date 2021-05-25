// Copyright (c) Iwer Petersen. All rights reserved.


#include "GeoLocatedActor.h"


#include "DrawDebugHelpers.h"

// Sets default values
AGeoLocatedActor::AGeoLocatedActor()
{
     // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>("RootComponent");
    GeoLocation = CreateDefaultSubobject<UGeoLocationComponent>("GeoLocation");
    SetRootComponent(Root);
}

// Called when the game starts or when spawned
void AGeoLocatedActor::BeginPlay()
{
    Super::BeginPlay();

}

// Called every frame
void AGeoLocatedActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

void AGeoLocatedActor::OnConstruction(const FTransform & Transform)
{
    Super::OnConstruction(Transform);
    GeoLocation->UpdateParentActorLocation();
}


