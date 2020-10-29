// Copyright (c) Iwer Petersen. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GeoRegion.h"
#include "GeoReferenceActor.generated.h"

UCLASS()
class GEOREFERENCE_API AGeoReferenceActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGeoReferenceActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGeoRegion Region;

    UFUNCTION(BlueprintCallable)
    FVector ToGameCoordinate(FVector geocoordinate);

    UFUNCTION(BlueprintCallable)
    FVector ToGeoCoordinate(FVector gamecoordinate);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;



};
