#include "SkyPosition.h"
#include <time.h>
#include <amp_math.h>

#define PI 3.14159265358979323846
#define DEGREES(a) (a * PI / 180.0)

double SkyPosition::ComputeSideralTime()
{
	tm the_time;
	__time64_t long_time;

	_time64(&long_time);
	_gmtime64_s(&the_time, &long_time);

	double year = the_time.tm_year + 1900.0;
	double month = the_time.tm_mon + 1.0;
	if (month < 3)
	{
		year -= 1;
		month += 12;
	}

	double c = floor(year / 100);
	double b = 2 - c + floor(c / 4);
	double t = (double)the_time.tm_hour / 24.0 + (double)the_time.tm_min / 1440.0 + (double)the_time.tm_sec / 86400.0;
	JulianDay = floor(365.25 * (year + 4716)) + floor(30.6001 * (month + 1)) + (double)the_time.tm_mday + t + b - 1524.5;

	double h = the_time.tm_hour + the_time.tm_min / 60.0 + the_time.tm_sec / 3600.0;
	double n = JulianDay - 2415384.5;
	double ts = 23750.3 + 236.555362 * n;
	double tsmh_h = ts / 3600.0 + h - (Longitude / 15.0);

	return fmod(tsmh_h, 24.0);
}

void SkyPosition::SetLocation(double longitude, double latitude, double elevation)
{
	Longitude = longitude;
	Latitude = latitude;
	Elevation = elevation;
}

void SkyPosition::SetEquatorialCoord(double ra, double dec)
{
	RightAscension = ra;
	Declination = dec;
	ComputeAzimuthalCoord();
}

void SkyPosition::ComputeEquatorialCoord()
{
	double azimuth = Azimuth - 180.0;

	double tsmh_h = ComputeSideralTime();

	double alt_rad = DEGREES(Altitude);
	double az_rad = DEGREES(azimuth);
	double lat_rad = DEGREES(Latitude);

	double sin_dec = sin(lat_rad) * sin(alt_rad) - cos(lat_rad) * cos(alt_rad) * cos(az_rad);
	double sin_ha = cos(alt_rad) * sin(DEGREES(azimuth)) / cos(asin(sin_dec));

	double hour_angle = asin(sin_ha) * 180.0 / PI;
	hour_angle /= 15.0;
	RightAscension = fmod(tsmh_h - hour_angle, 24.0);
	Declination = asin(sin_dec) * 180.0 / PI;
}

double SkyPosition::GetRightAscension()
{
	return RightAscension;
}

double SkyPosition::GetDeclination()
{
	return Declination;
}

void SkyPosition::SetAzimuthalCoord(double alt, double az)
{
	Altitude = alt;
	Azimuth = az;
	ComputeEquatorialCoord();
}

void SkyPosition::ComputeAzimuthalCoord()
{
	double dec_rad = DEGREES(Declination);
	double lat_rad = DEGREES(Latitude);

	double tsmh_h = ComputeSideralTime();
	double hour_angle = tsmh_h - RightAscension;

	double ah = DEGREES(hour_angle * 15.0);
	double cos_z = sin(lat_rad) * sin(dec_rad) + cos(lat_rad) * cos(dec_rad) * cos(ah);
	double z = acos(cos_z);
	double sin_a = cos(dec_rad) * sin(ah) / sin(z);

	Altitude = 90.0 - (z * 180.0 / PI);
	Azimuth = asin(sin_a) * 180.0 / PI + 180.0;
}

double SkyPosition::GetAltitude()
{
	return Altitude;
}

double SkyPosition::GetAzimuth()
{
	return Azimuth;
}
