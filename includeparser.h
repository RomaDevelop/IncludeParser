#ifndef INCLUDEPARSER_H
#define INCLUDEPARSER_H

#include <vector>

#include <QDialog>
#include <QMainWindow>

#include "filesitems.h"

QT_BEGIN_NAMESPACE
namespace Ui { class IncludeParser; }
QT_END_NAMESPACE

class IncludeParser : public QMainWindow
{
	Q_OBJECT

	std::vector<QWidget*> toSave;

public:
	IncludeParser(QWidget *parent = nullptr);
	~IncludeParser();
	void CreateContextMenu();

private:
	vectFilesItems vfi;
	void PrintVectFiles(int showCode);

private:
	QStringList releases;
	void EditReleases();
	QStringList GetReleasesAsMasks();
	void AddRelease(QString dir, bool showInputLineDialog = true);
	void RemoveUnexitingRealeses();

private slots:
	void SlotScan();

	void on_tableWidget_cellDoubleClicked(int row, int column);

	void on_pushButtonMassUpdate_clicked();


private:
	Ui::IncludeParser *ui;
	enum cols { colFileName = 0, colFilePathName = 1, colLastModified = 2};
};
#endif // INCLUDEPARSER_H
