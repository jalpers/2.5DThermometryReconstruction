#include "ThermometryReconstruction.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication q_application(argc, argv);
    ThermometryReconstruction q_application_window;
	/*
	*	Open and set stylesheet.
	*/
	QFile File("..\\style\\stylesheet.qss");
	File.open(QFile::ReadOnly);
	QString StyleSheet = QLatin1String(File.readAll());
	File.close();
	qApp->setStyleSheet(StyleSheet);
	/*
	*	Show the window and return the execute callback from the QApplication.
	*/
	q_application_window.show();
    return q_application.exec();
}
