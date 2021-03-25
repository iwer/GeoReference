// Copyright (c) Iwer Petersen. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GeoLocationComponent.generated.h"


class AGeoReferenceActor;
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

	AGeoReferenceActor* GeoRef;
	

	// Sets default values for this component's properties
	UGeoLocationComponent();
	FVector SnapToGround(const FVector& Vector, float Range);

	UFUNCTION(BlueprintCallable)
	void UpdateParentActorLocation();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
	void FindGeoReferenceActor();
		
};
