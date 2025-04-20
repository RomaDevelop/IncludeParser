#include "includeparser.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	vectFilesItems::TestCheckExcludeList();

	IncludeParser w;
	w.show();
	return a.exec();
}
