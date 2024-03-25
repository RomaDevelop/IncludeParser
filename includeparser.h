#ifndef INCLUDEPARSER_H
#define INCLUDEPARSER_H

#include <vector>
using namespace std;

#include <QDialog>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class IncludeParser; }
QT_END_NAMESPACE

class IncludeParser : public QMainWindow
{
	Q_OBJECT

	vector<QWidget*> toSave;
public:
	IncludeParser(QWidget *parent = nullptr);
	~IncludeParser();

private slots:
	void on_pushButtonScan_clicked();

	void on_tableWidget_cellDoubleClicked(int row, int column);

	void on_pushButtonMassUpdate_clicked();

private:
	Ui::IncludeParser *ui;
};
#endif // INCLUDEPARSER_H
