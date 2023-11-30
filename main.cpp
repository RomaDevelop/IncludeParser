#include "includeparser.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	IncludeParser w;
	w.show();
	return a.exec();
}
