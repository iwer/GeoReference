// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GeoLocationComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GEOREFERENCE_API UGeoLocationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float Longitude;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float Latitude;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		bool bSnapToGround;
	// Sets default values for this component's properties
	UGeoLocationComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void OnComponentCreated() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	FVector SnapToGround(const FVector& Vector, float Range);
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
	void UpdateParentActorLocation();
		
};
