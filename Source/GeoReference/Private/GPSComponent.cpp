// Fill out your copyright notice in the Description page of Project Settings.


#include "GPSComponent.h"

// Sets default values for this component's properties
UGPSComponent::UGPSComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGPSComponent::BeginPlay()
{
	Super::BeginPlay();

    // Find GeoReference if exists, quit if not
    for (TObjectIterator<AGeoReferenceActor> Itr; Itr; ++Itr)
    {
        if (Itr->IsA(AGeoReferenceActor::StaticClass())) {
            GeoRef = *Itr;
            break;
        }
        else {
            continue;
        }
    }
    if (!GeoRef) {
        UE_LOG(LogTemp, Error, TEXT("UGeoLocationComponent: No AGeoReferenceActor found!"))
            return;
    }
}


// Called every frame
void UGPSComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...

	FVector gameloc = GetOwner()->GetActorLocation();
    FVector geoloc;
    if (GeoRef) {
        // Transform to geo coordinates
        geoloc = GeoRef->ToGeoCoordinate(gameloc);
        Latitude = geoloc.X;
        Longitude = geoloc.Y;
    }

    
}

