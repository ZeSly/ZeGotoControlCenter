#ifndef GPSSTATUSFRAME_H
#define GPSSTATUSFRAME_H

#include <QBrush>
#include <QPen>
#include <QFrame>
#include <QWidget>
#include <QDateTime>

namespace Ui
{
	class GPSStatusFrame;
}

struct Satellite
{
	QString Id;
	int Elevation;
	int Azimuth;
	int SNR;
};

class GPSStatusFrame : public QFrame
{
	Q_OBJECT

public:
	explicit GPSStatusFrame(QWidget *parent = 0);
	void SetSatellitesInView(int);
	void AddSatellites(Satellite &);
	QString text;

protected:
	void paintEvent(QPaintEvent *event);

private:
	int SatellitesInView;
	QList<Satellite> Satellites;
	QList<Satellite> SatToDraw;
};

#endif // GPSSTATUSFRAME_H