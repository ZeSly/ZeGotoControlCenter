#pragma once
class SkyPosition
{
public:
	void SetLocation(double longitude, double latitude, double elevation);
	
	void SetEquatorialCoord(double ra, double dec);
	double GetRightAscension();
	double GetDeclination();
	
	void SetAzimuthalCoord(double alt, double az);
	double GetAltitude();
	double GetAzimuth();

private:
	double Latitude;
	double Longitude;
	double Elevation;

	double RightAscension;
	double Declination;

	double Altitude;
	double Azimuth;

	double JulianDay;
	double ComputeSideralTime();
	void ComputeAzimuthalCoord();
	void ComputeEquatorialCoord();
};

