#include <QPainter>
#include "GPSStatusFrame.h"

GPSStatusFrame::GPSStatusFrame(QWidget *parent)
	: QFrame(parent)
{
	setAutoFillBackground(true);
}

void GPSStatusFrame::SetSatellitesInView(int s)
{
	SatellitesInView = s;
}

void GPSStatusFrame::AddSatellites(Satellite &s)
{
	Satellites << s;
	if (Satellites.size() == SatellitesInView)
	{
		SatToDraw = Satellites;
		Satellites.clear();
		update();
	}
}

void GPSStatusFrame::paintEvent(QPaintEvent * event)
{
	const double pi = 3.14159265359;
	QPoint circle_center(width() - height() / 2, height() / 2);

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(QPen(QColor("#fff"), 0));
	QRect txt_rect = painter.boundingRect(0, 0, 500, 20, Qt::AlignRight, text);
	painter.drawText(0, 0, txt_rect.width(), txt_rect.height(), 0, text);
	int dd = txt_rect.height() + 10;
	painter.setPen(QPen(QColor("#555"), 0));
	painter.setBrush(QBrush(Qt::black));
	int radius = height() / 2 - 2;
	painter.drawEllipse(circle_center, radius, radius);
	radius -= 8;
	painter.drawEllipse(circle_center, radius, radius);
	QFont font = QFont();
	font.setPointSize(6);
	painter.setFont(font);
	txt_rect = painter.boundingRect(0, 0, 50, 50, Qt::AlignCenter, "360");
	for (int i = 1; i <= 12; i++)
	{
		painter.setPen(QPen(QColor("#555"), 0, Qt::DotLine));
		int x2 = sin(pi / 6.0 * i) * radius + circle_center.x();
		int y2 = -cos(pi / 6.0 * i) * radius + circle_center.y();
		painter.drawLine(circle_center.x(), circle_center.y(), x2, y2);
		int x = sin(pi / 6.0 * i) * (radius-8) + circle_center.x();
		int y = -cos(pi / 6.0 * i) * (radius-8) + circle_center.y();
		painter.setPen(QPen(QColor("#fff"), 0));
		painter.drawText(x - txt_rect.width() / 2, y - txt_rect.height() / 2, txt_rect.width(), txt_rect.height(),
			Qt::AlignCenter, QString("%1").arg(i * 30));
	}

	if (SatToDraw.size())
	{
		int bar_height = (height() - dd) / SatToDraw.size();
		int bar_width = circle_center.x() - height() / 2 - 4;
		foreach (const Satellite &sat, SatToDraw)
		{
			int x = sin(pi * (sat.Azimuth / 180.0)) * ((90.0 - (double)sat.Elevation) * ((double)radius / 90.0)) + circle_center.x();
			int y = -cos(pi * (sat.Azimuth / 180.0)) * ((90.0 - (double)sat.Elevation) * ((double)radius / 90.0)) + circle_center.y();
			if (sat.SNR > 0)
			{
				painter.setPen(QPen(QColor("#0f0"), 0));
			}
			else
			{
				painter.setPen(QPen(QColor("#f00"), 0));
			}
			QPoint sat_center(x, y);
			painter.drawPoint(sat_center);
			painter.setFont(font);
			painter.drawText(sat_center, sat.Id);

			painter.setPen(QPen(QColor("#555"), 0));
			painter.setBrush(QBrush(Qt::darkBlue));
			QRect rr = QRect(1, dd, bar_width * sat.SNR / 99, bar_height);
			painter.drawRect(rr);
			painter.setFont(QFont());
			painter.drawText(rr, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextDontClip, sat.Id);
			dd += bar_height;
		}
	}

}
