// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GeoLocationComponent.generated.h"


UCLASS( Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GEOREFERENCE_API UGeoLocationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GeoLocation")
		float Longitude;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GeoLocation")
		float Latitude;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GeoLocation")
		bool bSnapToGround;

	// Sets default values for this component's properties
	UGeoLocationComponent();
	FVector SnapToGround(const FVector& Vector, float Range);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	//virtual void OnRegister() override;
	//virtual void PostInitProperties() override;
	virtual void InitializeComponent() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
	void UpdateParentActorLocation();
		
};
