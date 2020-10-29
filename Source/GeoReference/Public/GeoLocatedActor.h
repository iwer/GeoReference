// Copyright (c) Iwer Petersen. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"

#include "GeoRegion.h"

#include "GeoLocatedActor.generated.h"

UCLASS()
class GEOREFERENCE_API AGeoLocatedActor : public AActor
{
	GENERATED_BODY()

    USceneComponent * root;
public:


    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGeoRegion Region;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Longitude;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Latitude;

	// Sets default values for this actor's properties
	AGeoLocatedActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    virtual void OnConstruction(const FTransform & Transform) override;

};
