// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ROI.h"
#include "GeoReferenceActor.h"
#include "GeoLocationComponent.h"
#include "Components/SceneComponent.h"
#include "Landscape.h"

#include "GeoLocatedActor.generated.h"

UCLASS()
class GEOREFERENCE_API AGeoLocatedActor : public AActor
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
    USceneComponent * Root;
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta=(ExposeFunctionCategories = "GeoLocation"))
    UGeoLocationComponent * GeoLocation;

    // Sets default values for this actor's properties
    AGeoLocatedActor();


protected:
    AGeoReferenceActor * GeoRef;
    ALandscape * Landscape;

    // Called when the game starts or when spawned
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform & Transform) override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

};
