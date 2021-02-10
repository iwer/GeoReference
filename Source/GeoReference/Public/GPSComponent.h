// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GeoReferenceActor.h"

#include "GPSComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GEOREFERENCE_API UGPSComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float Longitude;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float Latitude;

	// Sets default values for this component's properties
	UGPSComponent();

protected:
	AGeoReferenceActor* GeoRef;
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
