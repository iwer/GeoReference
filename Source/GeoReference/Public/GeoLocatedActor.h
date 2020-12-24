// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ROI.h"
#include "GeoReferenceActor.h"
#include "Components/SceneComponent.h" 
#include "Landscape.h"

#include "GeoLocatedActor.generated.h"

UCLASS()
class GEOREFERENCE_API AGeoLocatedActor : public AActor
{
	GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    USceneComponent * Root;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float Longitude;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float Latitude;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bSnapToGround;

	// Sets default values for this actor's properties
	AGeoLocatedActor();


protected:
    AGeoReferenceActor * GeoRef;
    ALandscape * Landscape;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform & Transform) override;

    FVector SnapToGround(const FVector &Vector, float Range);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
