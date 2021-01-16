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


int FGeoReferenceHelper::GetUTMZone(double Lon, double Lat)
{
    int utmzone = floor((Lon + 180) / 6) + 1;

    if (Lat >= 56.0 && Lat < 64.0 && Lon >= 3.0 && Lon < 12.0)
        utmzone = 32;

    // Special zones for Svalbard
    if (Lat >= 72.0 && Lat < 84.0) {
        if (Lon >= 0.0 && Lon < 9.0) {
            utmzone = 31;
        }
        else if (Lon >= 9.0 && Lon < 21.0) {
            utmzone = 33;
        }
        else if (Lon >= 21.0 && Lon < 33.0) {
            utmzone = 35;
        }
        else if (Lon >= 33.0 && Lon < 42.0) {
            utmzone = 37;
        }
    }
    return utmzone;
}

char FGeoReferenceHelper::GetUTMLetter(double Lat)
{
    char LetterDesignator;

    if ((84 >= Lat) && (Lat >= 72)) LetterDesignator = 'X';
    else if ((72 > Lat) && (Lat >= 64)) LetterDesignator = 'W';
    else if ((64 > Lat) && (Lat >= 56)) LetterDesignator = 'V';
    else if ((56 > Lat) && (Lat >= 48)) LetterDesignator = 'U';
    else if ((48 > Lat) && (Lat >= 40)) LetterDesignator = 'T';
    else if ((40 > Lat) && (Lat >= 32)) LetterDesignator = 'S';
    else if ((32 > Lat) && (Lat >= 24)) LetterDesignator = 'R';
    else if ((24 > Lat) && (Lat >= 16)) LetterDesignator = 'Q';
    else if ((16 > Lat) && (Lat >= 8)) LetterDesignator = 'P';
    else if ((8 > Lat) && (Lat >= 0)) LetterDesignator = 'N';
    else if ((0 > Lat) && (Lat >= -8)) LetterDesignator = 'M';
    else if ((-8 > Lat) && (Lat >= -16)) LetterDesignator = 'L';
    else if ((-16 > Lat) && (Lat >= -24)) LetterDesignator = 'K';
    else if ((-24 > Lat) && (Lat >= -32)) LetterDesignator = 'J';
    else if ((-32 > Lat) && (Lat >= -40)) LetterDesignator = 'H';
    else if ((-40 > Lat) && (Lat >= -48)) LetterDesignator = 'G';
    else if ((-48 > Lat) && (Lat >= -56)) LetterDesignator = 'F';
    else if ((-56 > Lat) && (Lat >= -64)) LetterDesignator = 'E';
    else if ((-64 > Lat) && (Lat >= -72)) LetterDesignator = 'D';
    else if ((-72 > Lat) && (Lat >= -80)) LetterDesignator = 'C';
    else LetterDesignator = 'Z'; //This is here as an error flag to show that the Lat is outside the UTM limits

    return LetterDesignator;
}

int FGeoReferenceHelper::GetEPSGForUTM(int utmZone, bool bNorthernHemi)
{
    int epsgCode = -1;
    if(utmZone != -1) {
        epsgCode = 32600;
        epsgCode += utmZone;
        if(!bNorthernHemi)
            epsgCode += 100;
    }
    return epsgCode;
}
