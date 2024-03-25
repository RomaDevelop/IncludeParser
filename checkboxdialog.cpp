#include "checkboxdialog.h"

#include <memory>
using namespace std;

#include <QDebug>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>

CheckBoxDialog::CheckBoxDialog(QWidget *parent) : QDialog(parent)
{

}

bool CheckBoxDialog::Execute(const QStringList &values, const std::vector<bool> &startCheched, const std::vector<bool> &enabled)
{
	allValues.clear();
	checkedValues.clear();
	checkedIndexes.clear();
	enabledIndexes.clear();

	auto loV = make_shared<QVBoxLayout>(this);
	auto loH = make_shared<QHBoxLayout>(this);
	auto lw = make_shared<QListWidget>(this);
	auto chAl = make_shared<QCheckBox>(this);
	auto chNo = make_shared<QCheckBox>(this);

	auto dbb = make_shared<QDialogButtonBox>(this);
	auto btnOk = make_shared<QPushButton>("Ok", this);
	auto btnCa = make_shared<QPushButton>("Отмена", this);
	connect(btnOk.get(),&QPushButton::clicked,[this](){ action = "Ok"; this->hide(); });
	connect(btnCa.get(),&QPushButton::clicked,[this](){ this->hide();});

	QListWidget *lwpt = lw.get();	// в лямбды коннекта нужно передавать обычные указатели, а не смарт, иначе вылетают криты при закрытии
	QCheckBox *chAlpt = chAl.get();
	QCheckBox *chNopt = chNo.get();

	chAl->setChecked(true);
	connect(chAl.get(),&QCheckBox::clicked,[lwpt, chAlpt](){
		chAlpt->setChecked(true);
		for(int i=0; i<lwpt->count(); i++)
			lwpt->item(i)->setCheckState(Qt::Checked);
	});

	chNo->setChecked(false);
	connect(chNo.get(),&QCheckBox::clicked,[lwpt, chNopt](){
		chNopt->setChecked(false);
		for(int i=0; i<lwpt->count(); i++)
			lwpt->item(i)->setCheckState(Qt::Unchecked);
	});

	this->setLayout(loV.get());
	loV->addLayout(loH.get());
	loH->addWidget(chAl.get());
	loH->addWidget(chNo.get());
	loH->addStretch();
	loV->addWidget(lw.get());
	loV->addWidget(dbb.get());
	dbb->addButton(btnOk.get(),QDialogButtonBox::AcceptRole);
	dbb->addButton(btnCa.get(),QDialogButtonBox::AcceptRole);

	for(auto str:values)
	{
		lw->addItem(str);
		lw->item(lw->count()-1)->setCheckState(Qt::Unchecked);
	}

	for(int i=0; i<lw->count() && i<(int)startCheched.size(); i++)
		if(startCheched[i]) lw->item(i)->setCheckState(Qt::Checked);

	for(int i=0; i<lw->count() && i<(int)enabled.size(); i++)
		if(startCheched[i]) lw->item(i)->setFlags(lw->item(i)->flags() ^ Qt::ItemIsEnabled);

	this->exec();

	bool retVal = false;
	if(action == "Ok")
	{
		retVal = true;
		allValues = values;
		enabledIndexes = enabled;
		for(int i=0; i<lw->count(); i++)
			if(lw->item(i)->checkState() == Qt::Checked)
			{
				checkedValues += lw->item(i)->text();
				checkedIndexes.push_back(i);
			}
	}

	return retVal;
}

const QStringList & CheckBoxDialog::GetAllValues() { return allValues; }
const QStringList & CheckBoxDialog::GetCheckedValues() { return checkedValues; }
const std::vector<bool> & CheckBoxDialog::GetCheckedIndexes() { return checkedIndexes; }
const std::vector<bool> & CheckBoxDialog::GetEnabledIndexes() { return enabledIndexes; }
