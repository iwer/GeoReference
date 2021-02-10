// Fill out your copyright notice in the Description page of Project Settings.


#include "GeoLocationComponent.h"
#include "GeoReferenceActor.h"

// Sets default values for this component's properties
UGeoLocationComponent::UGeoLocationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGeoLocationComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UGeoLocationComponent::OnComponentCreated()
{
    // Find GeoReference if exists, quit if not
    AGeoReferenceActor* GeoRef = nullptr;
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

    // Transform to game coordinates
    FVector Location = GeoRef->ToGameCoordinate(FVector(Longitude, Latitude, 0));

    // If SnapToLandscape is enabled and there is a landscape
    if (bSnapToGround) {
        Location = SnapToGround(Location, 100000);

    }

    GetOwner()->SetActorLocation(Location);
}


// Called every frame
void UGeoLocationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

FVector UGeoLocationComponent::SnapToGround(const FVector& Vector, float Range)
{
    FVector HitLocation(Vector.X, Vector.Y, 0);
    // line trace to find z
    FHitResult Hit(ForceInit);
    FVector Start = Vector + FVector(0, 0, Range);
    FVector End = Vector + FVector(0, 0, -1 * Range);
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(GetOwner());

    //DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 5.f, false, 4.f);

    GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldDynamic, CollisionParams);
    if (Hit.IsValidBlockingHit()) {
        HitLocation.Z = Hit.Location.Z;
    }
    else {
        HitLocation.Z = Vector.Z;
    }
    return HitLocation;
}