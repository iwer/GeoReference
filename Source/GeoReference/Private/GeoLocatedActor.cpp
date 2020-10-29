// Copyright (c) Iwer Petersen. All rights reserved.


#include "GeoLocatedActor.h"
#include "GeoReference.h"

// Sets default values
AGeoLocatedActor::AGeoLocatedActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    root = CreateDefaultSubobject<USceneComponent>(TEXT("sceneRoot"));
    //root->SetWorldScale3D(FVector(100,100,100));
    SetRootComponent(root);

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

    // UE_LOG(LogTemp, Warning, TEXT("AGeoLocatedActor::OnConstruction Region: %f, %f, %f, %f"), Region.West, Region.East, Region.South, Region.North);
    //
    // FVector location = FGeoReference::ToGameCoordinate(Longitude, Latitude, Region.West, Region.North, Region.South);
    //
    // //FHitResult hitResult;
    // //FVector rayStart(location.X, location.Y, 1000000);
    // //FVector rayEnd(location.X, location.Y, -100000);
    // //GetWorld()->LineTraceSingleByChannel(hitResult, rayStart, rayEnd, ECollisionChannel::ECC_Visibility);
    //
    // //FVector impactPoint = hitResult.ImpactPoint;
    // //location.Z = impactPoint.Z;
    //
    // root->SetWorldLocation(location);
}
