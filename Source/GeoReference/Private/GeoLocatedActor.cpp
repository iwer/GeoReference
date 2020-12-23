// Fill out your copyright notice in the Description page of Project Settings.


#include "GeoLocatedActor.h"


#include "DrawDebugHelpers.h"

// Sets default values
AGeoLocatedActor::AGeoLocatedActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
    bSnapToGround = true;
    GeoRef = nullptr;
    Landscape = nullptr;

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
    // Find GeoReference if exists, quit if not
    for (TObjectIterator<AGeoReferenceActor> Itr; Itr; ++Itr)
    {
        if(Itr->IsA(AGeoReferenceActor::StaticClass())){
            GeoRef = *Itr;
            break;
        } else {
            continue;
        }
    }
    if(!GeoRef){
        UE_LOG(LogTemp, Error, TEXT("AGeoLocatedActor: No AGeoReferenceActor found!"))
        return;
    }

    // Transform to game coordinates
    FVector Location = GeoRef->ToGameCoordinate(FVector(Longitude, Latitude, 0));

    // If SnapToLandscape is enabled and there is a landscape
    if(bSnapToGround) {
        // line trace to find z
        FHitResult Hit(ForceInit);
        FVector Start = Location + FVector(0,0,100000);
        FVector End = Location + FVector(0,0,-100000);
        FCollisionQueryParams CollisionParams;
        CollisionParams.AddIgnoredActor(this);

        DrawDebugLine(GetWorld(), Start, End, FColor::Green, true, 2.f, false, 4.f);

        GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldDynamic, CollisionParams);
        if(Hit.IsValidBlockingHit()) {
            Location.Z = Hit.Location.Z;
        } else {
            Location.Z = Transform.GetTranslation().Z;
        }

    } else {
        Location.Z = Transform.GetTranslation().Z;
    }
    // UE_LOG(LogTemp, Warning, TEXT("AGeoLocatedActor: Game-Location: %s"), *Location.ToString())

    SetActorLocation(Location);

}
