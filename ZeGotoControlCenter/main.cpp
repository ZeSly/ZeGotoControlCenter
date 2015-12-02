#include "singleapplication.h"
#include "zegotocontrolcenter.h"
#include <QtWidgets/QApplication>
#include <QFile>
#include <QObject>

int main(int argc, char *argv[])
{
	QApplication::setApplicationName("OpenGoto Control Center");
	QApplication::setOrganizationName("ZeSly");

	//#ifdef _DEBUG
	//    QFile File("Resources/darkstyle.css");
	//#else
	//    QFile File(":/Style/Resources/darkstyle.css");
	//#endif
	//    File.open(QFile::ReadOnly);
	//    QString StyleSheet = QLatin1String(File.readAll());

	SingleApplication a(argc, argv);
	//a.setStyleSheet(StyleSheet);
	ZeGotoControlCenter w;
	//QObject::connect(QApplication::instance(), SIGNAL(showUp()), &w, SLOT(raise()));
	w.show();
	return a.exec();
}
