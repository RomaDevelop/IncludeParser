#include "includeparser.h"
#include "ui_includeparser.h"

#include <windows.h>

#include <memory>
#include <algorithm>
#include <vector>
using namespace std;

#include <QLineEdit>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QDirIterator>
#include <QDateTime>
#include <QMessageBox>
#include <QCheckBox>
#include <QVBoxLayout>

#include "MyQFileDir.h"
#include "MyQShortings.h"
#include "MyQShellExecute.h"
#include "MyQStr.h"
#include "MyQDifferend.h"

#include "checkboxdialog.h"
#include "filesitems.h"

vectFilesItems vfi;

IncludeParser::IncludeParser(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::IncludeParser)
{
	ui->setupUi(this);
	this->move(30,30);

	toSave.push_back(ui->textEditScan);
	toSave.push_back(ui->lineEditExts);
	toSave.push_back(ui->textEditExeptFName);
	toSave.push_back(ui->textEditExeptFPath);
	toSave.push_back(ui->checkBoxHideIfOne);

	QAction *mShowInExplorer = new QAction("Показать в проводнике", ui->tableWidget);
	ui->tableWidget->addAction(mShowInExplorer);
	ui->tableWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
	connect(mShowInExplorer, &QAction::triggered,
			[this](){ ShellExecutePath(ui->tableWidget->item(ui->tableWidget->currentRow(),1)->text()); });

	QDir pathBackup(QFileInfo(QCoreApplication::applicationFilePath()).path() + "/files");
	if(!pathBackup.exists()) pathBackup.mkdir(pathBackup.path());
	pathBackup.setPath(pathBackup.path() + "/backup");
	if(!pathBackup.exists()) pathBackup.mkdir(pathBackup.path());
	if(!pathBackup.exists())
	{
		QMessageBox::information(this, "Ошибка", "Ошибка создания директории для файла резервного копирования. Резервные копии не будут сохраняться!");
		return;
	}
	vfi.backupPath = pathBackup.path();

	QString settingsFile = QFileInfo(QCoreApplication::applicationFilePath()).path() + "/files/settings.stgs";
	QStringList pustishka;
	if(!mqd::LoadSettings(settingsFile, toSave, pustishka))
		QMb(this,"Ошибка чтения настроек", "Не удалось загрузить настройки, будут установлены по умолчанию");
}

IncludeParser::~IncludeParser()
{
	QString pathFiles = mqd::GetPathToExe()+"/files";
	if(!MQFD::CreatePath(pathFiles))
		QMessageBox::information(this, "Ошибка", "Ошибка создания директории для файла настроек, невозможно сохранить настройки");

	QString settingsFile = pathFiles + "/settings.stgs";
	if(!mqd::SaveSettings(settingsFile, toSave, {}))
		QMb(this,"Error", "Не удалось сохранить файл настроек "+settingsFile);

	delete ui;
}

void IncludeParser::on_pushButtonScan_clicked()
{
	vfi.ScanFiles(ui->textEditScan->toPlainText().split("\n"),
			  ui->lineEditExts->text().split(";"),
			  ui->textEditExeptFName->toPlainText().split("\n"),
			  ui->textEditExeptFPath->toPlainText().split("\n"),
			  ui->checkBoxHideIfOne->isChecked());

	vfi.PrintVectFiles(ui->tableWidget);
}

void IncludeParser::on_tableWidget_cellDoubleClicked(int row, int column)
{
	QTableWidgetItem *item = ui->tableWidget->item(row, column);

	FilesItems *filesFind {nullptr};
	FileItem *fileFind {nullptr};

	for(uint i=0; i<vfi.vectFiles.size(); i++)
	{
		for(uint j=0; j<vfi.vectFiles[i].filesItems.size(); j++)
			if(vfi.vectFiles[i].filesItems[j].itemFile == item || vfi.vectFiles[i].filesItems[j].itemModif == item)
			{
				filesFind = &vfi.vectFiles[i];
				fileFind = &vfi.vectFiles[i].filesItems[j];
			}
	}

	if(!filesFind || !fileFind) return;

	QMessageBox messageBox(QMessageBox::Question, "Замена файлов", "Вы кликнули на файл " + fileFind->info.filePath() + ".\n\nЧто нужно сделать?");
	QString replaceHimByNewest = "    Заменить его новейшим    ";
	QString replaceAllByHim = "    Заменить им все " + filesFind->name+"    ";
	QString replaceNothing = "  Ничего  ";
	if(fileFind->itemFile->backgroundColor() == vfi.colorNew) //порядок кнопок
	{
		messageBox.addButton(replaceAllByHim,QMessageBox::YesRole);
		messageBox.addButton(replaceHimByNewest,QMessageBox::YesRole);
	}
	else
	{
		messageBox.addButton(replaceHimByNewest,QMessageBox::YesRole);
		messageBox.addButton(replaceAllByHim,QMessageBox::YesRole);
	}
	messageBox.addButton(replaceNothing,QMessageBox::NoRole);
	int desision =  messageBox.exec();

	if(messageBox.buttons()[desision]->text() == replaceAllByHim) // кликнутый копируем на место всех
	{
		QFileInfoList files = filesFind->GetQFileInfoList();

		// берем все файлы кроме нашаего
		QFileInfoList filesToReplace;
		for(auto &f:files)
			if(f.filePath() != fileFind->info.filePath())
				filesToReplace += f;

		if(!filesToReplace.empty())
			MQFD::ReplaceFilesWithBackup(filesToReplace,fileFind->info, vfi.backupPath);
	}
	else if(messageBox.buttons()[desision]->text() == replaceHimByNewest) // кликнутый заменяем самым новым
	{
		QFileInfo newestModifFI = MQFD::GetNewestFI(filesFind->GetQFileInfoList());

		// если наш файл новее новейшего - неправльно определён новейший
		if(fileFind->info.lastModified() > newestModifFI.lastModified()) QMb(this,"Ошибка","Ошибка desision == 1. Код ошибки 505");

		if(fileFind->info.lastModified() == newestModifFI.lastModified()) QMb(this,"Замена файлов", "Данный файл является новейшим");
		else
		{
			if(QMessageBox::question(this, "Замена файла", "Заменить файл:\n" + fileFind->info.filePath() + "\n\nфайлом:\n"
									 + newestModifFI.filePath() + "?\n(Резервные копии будут сохранены)",
									 QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
				MQFD::ReplaceFileWhithBacup(newestModifFI, fileFind->info, vfi.backupPath);
		}
	}
	else if(messageBox.buttons()[desision]->text() == replaceNothing) ;  // Ничего
	else QMb(this,"Error","Error code 50002");

	on_pushButtonScan_clicked();
}

void IncludeParser::on_pushButtonMassUpdate_clicked()
{
	CheckBoxDialog *chDial = new CheckBoxDialog;
	QStringList values;

	for(auto &f:vfi.vectFiles)
		if(f.needUpdate)
			values += f.name;

	chDial->Execute(values);

	QStringList chValues = chDial->GetCheckedValues();

	for(int i=0; i<chValues.size(); i++)
	{
		for(auto &f:vfi.vectFiles)
			if(chValues[i] == f.name)
				f.UpdateFiles();
	}

	delete chDial;

	on_pushButtonScan_clicked();
}
