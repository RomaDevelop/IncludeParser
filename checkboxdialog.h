#ifndef CHECKBOXDIALOG_H
#define CHECKBOXDIALOG_H

#include <vector>

#include <QDialog>
#include <QStringList>

class CheckBoxDialog : public QDialog
{
	Q_OBJECT
	QString action;

	QStringList allValues;
	QStringList checkedValues;
	std::vector<bool> checkedIndexes;
	std::vector<bool> enabledIndexes;

public:
	explicit CheckBoxDialog(QWidget *parent = nullptr);

	bool Execute(const QStringList &values, const std::vector<bool> &startCheched = {}, const std::vector<bool> &enabled = {});

	const QStringList & GetAllValues();
	const QStringList & GetCheckedValues();
	const std::vector<bool> & GetCheckedIndexes();
	const std::vector<bool> & GetEnabledIndexes();
};

#endif // CHECKBOXDIALOG_H
