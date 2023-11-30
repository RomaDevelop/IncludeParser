#include "includeparser.h"
#include "ui_includeparser.h"

#include <algorithm>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QDirIterator>
#include <QDateTime>
#include <QMessageBox>
#include <QCheckBox>

#include "ShortingsQT.hpp"

IncludeParser::IncludeParser(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::IncludeParser)
{
	ui->setupUi(this);
	this->move(30,30);

	toSave.push_back(ui->lineEditIncludePath);

	QFile file(QFileInfo(QCoreApplication::applicationFilePath()).path() + "/files/settings.stgs");
	if(!file.exists()) QMessageBox::information(this, "Ошибка", "Отсутствует файл настроек "+file.fileName()+" Установлены по умолчанию.");
	else
	{
		QString Settings;
		file.open(QIODevice::ReadOnly);
		Settings = file.readAll();
		QString save_version = Settings.left(Settings.indexOf("[endSetting]"));
		if(save_version.indexOf("save_version 002") != -1)
		{
			QString spl="[endSetting]";
			int count=Settings.count(spl);
			for(int i=1; i<count; i++)
			{
				QString setting = GetElemetFromStrings(Settings,spl,i);
				QString name = GetElemetFromStrings(setting,"[s;]",1);
				QString className = GetElemetFromStrings(setting,"[s;]",2);
				QString value = GetElemetFromStrings(setting,"[s;]",3);

				QWidget *component = this->findChild<QWidget*>(name);
				if(component!=NULL)
				{
					if(className=="QLineEdit") 			((QLineEdit*)component)->setText(value);
				}
			}
		}
		else QMb(this,"Ошибка чтения настроек.", "Не известная версия сохранения");
	}
}

IncludeParser::~IncludeParser()
{
	QDir pathSave(QFileInfo(QCoreApplication::applicationFilePath()).path() + "/files");
	if(!pathSave.exists()) pathSave.mkdir(pathSave.path());\
	if(!pathSave.exists()) { QMessageBox::information(this, "Ошибка", "Ошибка создания директории для файла настроек, невозможно сохранить настройки"); return; }
	QFile file(pathSave.path() + "/settings.stgs");
	QString Settings = "save_version 002[endSetting]\n";
	for(unsigned int i=0; i<toSave.size(); i++)
		{
		QString class_name=toSave[i]->metaObject()->className();
		QString name=toSave[i]->objectName();

		if(class_name=="QCheckBox") Settings += "toSave[s;]"+name+"[s;]"+class_name+"[s;]"+QString::number(static_cast<QCheckBox*>(toSave[i])->isChecked())+"[s;]";
		if(class_name=="QLineEdit") Settings += "toSave[s;]"+name+"[s;]"+class_name+"[s;]"+static_cast<QLineEdit*>(toSave[i])->text()+"[s;]";
		Settings += "[endSetting]";
		}
	file.open(QIODevice::WriteOnly);
	file.write(Settings.toUtf8());
	delete ui;
}


void IncludeParser::on_pushButton_clicked()
{
	ui->lineEditIncludePath->setText(ui->lineEditIncludePath->text().replace("\\","/"));
	ui->textBrowser->clear();
	ui->textBrowser_2->clear();

	ui->tableWidget->clear();
	ui->tableWidget->setColumnCount(4);

	QDir dirLeft(ui->lineEditIncludePath->text());

	QFileInfoList dirContent = dirLeft.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

	for(auto &d:dirContent)
	{
		ui->textBrowser->append(d.fileName());
		int insInd = ui->tableWidget->rowCount();
		ui->tableWidget->insertRow(insInd);
		ui->tableWidget->setItem(insInd, 0, new QTableWidgetItem(d.fileName()));
		ui->tableWidget->setItem(insInd, 1, new QTableWidgetItem(d.lastModified().toString("yyyy.MM.dd hh:mm:ss")));
		
		
		//https://evileg.com/ru/post/78/
	}

	QDir dirRight = dirLeft;
	dirRight.cdUp();
	QDirIterator it(dirRight.path(), QStringList() << "*.h", QDir::NoFilter, QDirIterator::Subdirectories);
	while (it.hasNext())
	{
		QFileInfo f(it.next());

		if(f.filePath().indexOf("build-") == -1 &&
				ui->textBrowser->toPlainText().indexOf(f.fileName()) != -1 &&
				f.filePath().indexOf(dirLeft.path()) == -1)
			ui->textBrowser_2->append(f.filePath());
	}


}

void IncludeParser::on_pushButton_2_clicked()
{
	//for(int i=0; i<7; i++)
	//	qDebug() << GetElemetFromStrings("1|2|3|4|5|6|7|","|",i);

	for(int i=0; i<7; i++)
		qDebug() << GetElemetFromStrings("111||22||33333|444|55|66|77|","|",i);

	for(int i=0; i<7; i++)
		qDebug() << GetElemetFromStrings("111||22||33333|444|55|66|77|","||",i);

	for(int i=0; i<7; i++)
		qDebug() << GetElemetFromStrings("111||22||33333|444|55|66|77||||||||||||||||||","||",i);
}
