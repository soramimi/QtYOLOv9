
#include <QApplication>
#include "MainWindow.h"

int main(int argc, char **argv)
{
	putenv("QT_ASSUME_STDERR_HAS_CONSOLE=1");

	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();
}
