// Copyright (c) Iwer Petersen. All rights reserved.

#include "GeoReferenceHelper.h"

bool FGeoReferenceHelper::IsWGS84(OGRSpatialReferenceH ref)
{
	if (OSRIsGeographic(ref)) {
		auto geocs = OSRGetAttrValue(ref, "GEOGCS", 0);
		if (strncmp(geocs, "WGS 84", 6) == 0) {
			return true;
		}
	}
	return false;
}

bool FGeoReferenceHelper::IsUTM(OGRSpatialReferenceH ref)
{
	if (OSRIsProjected(ref)) {
		int north;
		int utmzone = OSRGetUTMZone(ref, &north);
		if (utmzone != 0) {
			return true;
		}
	}
	return false;
}

