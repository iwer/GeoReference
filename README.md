GeoReference - UE4 Plugin
===========================

This plugin provides a reference between game coordinates and geo-coordinates. The core of the concept is the GeoRegion which defines latitude and longitude of the world origin. Additionally a map size in meter is given to calculate border coordinates in WSG84 and UTM. This information can be used to get georeferenced objects to the correct position or to define a region of interest (ROI) for ressource acquisition.

## BP_GeoReferenceActor
Has to placed in the world to define the GeoReference. Is used by BP_GeoLocatedActors to caclulate game-coordinates.

## BP_GeoLocatedActor
Can be used to derive child blueprints which can be placed at a geo-coordinate which is automatically translated to game-coordinates. Needs a BP_GeoReferenceActor placed in the world.
