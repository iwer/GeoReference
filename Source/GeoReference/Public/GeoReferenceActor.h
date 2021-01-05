// Copyright (c) Iwer Petersen. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ROI.h"
#include "GeoReferenceActor.generated.h"

UCLASS()
class GEOREFERENCE_API AGeoReferenceActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGeoReferenceActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Longitude;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Latitude;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SizeM;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	URegionOfInterest * ROI;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bShowBounds;

    UFUNCTION(BlueprintCallable)
    FVector ToGameCoordinate(FVector geocoordinate);

    UFUNCTION(BlueprintCallable)
    FVector ToGeoCoordinate(FVector gamecoordinate);

    UFUNCTION(BlueprintCallable)
    bool IsGameCoordInsideROI(FVector gamecoord);

    UFUNCTION(BlueprintCallable)
    bool IsGeoCoordInsideROI(FVector geocoord);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
    virtual void OnConstruction(const FTransform & Transform) override;

private:
    UGeoCoordinate CalculateGeoLocation(FVector gamecoordinate);

};
