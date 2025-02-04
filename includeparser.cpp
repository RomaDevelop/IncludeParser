#include "includeparser.h"
#include "ui_includeparser.h"

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
#include "MyQExecute.h"
#include "MyQDifferent.h"
#include "MyQDialogs.h"

void ReplaceSleshesInTextEdit(QTextEdit *textEdit)
{
	auto text = textEdit->toPlainText();
	text.replace('\\','/');
	textEdit->setPlainText(text);
}

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
	toSave.push_back(ui->checkBoxHideUpdated);
	toSave.push_back(ui->chBoxExeptReleases);

	CreateContextMenu();

	QDir pathBackup(QFileInfo(QCoreApplication::applicationFilePath()).path() + "/files");
	if(!pathBackup.exists()) pathBackup.mkdir(pathBackup.path());
	pathBackup.setPath(pathBackup.path() + "/backup");
	if(!pathBackup.exists()) pathBackup.mkdir(pathBackup.path());
	if(!pathBackup.exists())
	{
		QMessageBox::information(this, "Ошибка",
								 "Ошибка создания директории для файла резервного копирования. Резервные копии не будут сохраняться!");
		return;
	}
	vfi.backupPath = pathBackup.path();

	connect(ui->btnExeptReleases, &QPushButton::clicked, this, &IncludeParser::EditReleases);

	QString settingsFile = QFileInfo(QCoreApplication::applicationFilePath()).path() + "/files/settings.stgs";
	if(!MyQDifferent::LoadSettings(settingsFile, toSave, releases))
		QMb(this,"Ошибка чтения настроек", "Не удалось загрузить настройки, будут установлены по умолчанию");
}

IncludeParser::~IncludeParser()
{
	ReplaceSleshesInTextEdit(ui->textEditScan);
	ReplaceSleshesInTextEdit(ui->textEditExeptFName);
	ReplaceSleshesInTextEdit(ui->textEditExeptFPath);

	QString pathFiles = MyQDifferent::PathToExe()+"/files";
	if(!QDir().mkpath(pathFiles))
		QMessageBox::information(this, "Ошибка", "Ошибка создания директории для файла настроек, невозможно сохранить настройки");

	QString settingsFile = pathFiles + "/settings.stgs";
	if(!MyQDifferent::SaveSettings(settingsFile, toSave, GetReleases()))
		QMb(this,"Error", "Не удалось сохранить файл настроек "+settingsFile);

	delete ui;
}

void IncludeParser::CreateContextMenu()
{
	ui->tableWidget->setContextMenuPolicy(Qt::ActionsContextMenu);

	QAction *mShowInExplorer = new QAction("Показать в проводнике", ui->tableWidget);
	ui->tableWidget->addAction(mShowInExplorer);
	connect(mShowInExplorer, &QAction::triggered,[this](){
		MyQExecute::ShowInExplorer(ui->tableWidget->item(ui->tableWidget->currentRow(),colFilePathName)->text());
	});

	QAction *mAddInReleases = new QAction("Добавить в исключения выпусков", ui->tableWidget);
	ui->tableWidget->addAction(mAddInReleases);
	connect(mAddInReleases, &QAction::triggered, this, &IncludeParser::AddRelease);
}

void IncludeParser::PrintVectFiles(int showCode)
{
	ui->tableWidget->setRowCount(0);
	ui->tableWidget->clear();
	ui->tableWidget->setColumnCount(3);
	ui->tableWidget->setColumnWidth(colFileName, ui->tableWidget->width()*0.30);
	ui->tableWidget->setColumnWidth(colFilePathName, ui->tableWidget->width()*0.50);
	ui->tableWidget->setColumnWidth(colLastModified, ui->tableWidget->width()*0.15);

	int index=0;
	int indexRight = 0;
	for(auto &vf:vfi.vectFiles)
	{
		if(showCode == vectFilesItems::showAll) {}
		else if(showCode == vectFilesItems::showNeedUpdate) { if(!vf.needUpdate) continue; }
		else { QMbc(nullptr, "wrong showCode", "wrong showCode"); }

		while(index >= ui->tableWidget->rowCount()) ui->tableWidget->insertRow(index);
		ui->tableWidget->setItem(index, colFileName, new QTableWidgetItem(vf.name));
		vf.item = ui->tableWidget->item(index,0);
		indexRight = index;
		index++;
		for(auto &f:vf.filesItems)
		{
			while(indexRight >= ui->tableWidget->rowCount()) ui->tableWidget->insertRow(indexRight);
			ui->tableWidget->setItem(indexRight, colFilePathName, new QTableWidgetItem(f.info.filePath()));
			ui->tableWidget->setItem(indexRight, colLastModified,
									 new QTableWidgetItem(f.info.lastModified().toString(vectFilesItems::dateFormat)));
			f.itemFile  = ui->tableWidget->item(indexRight,colFilePathName);
			f.itemModif = ui->tableWidget->item(indexRight,colLastModified);
			if(f.needUpdate)
			{
				f.itemFile->setBackgroundColor(vectFilesItems::colorOld);
				f.itemModif->setBackgroundColor(vectFilesItems::colorOld);
			}
			else
			{
				f.itemFile->setBackgroundColor(vectFilesItems::colorNew);
				f.itemModif->setBackgroundColor(vectFilesItems::colorNew);
			}
			indexRight++;
		}
		if(indexRight > index) index = indexRight;
	}
}

void IncludeParser::EditReleases()
{
	auto tableUptr = MyQDialogs::Table({GetReleases()}, {"Путь к дистрибутиву"});
	QStringList newReleases;
	for(int row = 0; row<tableUptr->rowCount(); row++)
	{
		newReleases += tableUptr->item(row, 0)->text();
	}
	releases = std::move(newReleases);
	RemoveUnexitingRealeses();
}

const QStringList & IncludeParser::GetReleases()
{
	RemoveUnexitingRealeses();
	return releases;
}

void IncludeParser::AddRelease()
{
	QString textFromTable = ui->tableWidget->item(ui->tableWidget->currentRow(),colFilePathName)->text();
	QString release = MyQDialogs::InputText("Укжите путь к дистрибутиву", textFromTable, 700, 200);
	if(!QFileInfo(release).isDir()) { QMbi(this, "Ошибка", "Указанное значение не является каталогом"); return; }
	if(releases.contains(release)) { QMbi(this, "Ошибка", "Указанное значение уже добавлено"); return; }

	release.replace("\\", "/");
	releases += release;

	on_pushButtonScan_clicked();
}

void IncludeParser::RemoveUnexitingRealeses()
{
	auto removeRes = std::remove_if(releases.begin(), releases.end(), [](const QString &release){ return !QFileInfo(release).isDir();});
	releases.erase(removeRes, releases.end());
}

void IncludeParser::on_pushButtonScan_clicked()
{
	ReplaceSleshesInTextEdit(ui->textEditScan);
	ReplaceSleshesInTextEdit(ui->textEditExeptFName);
	ReplaceSleshesInTextEdit(ui->textEditExeptFPath);

	QStringList pathsExept = ui->textEditExeptFPath->toPlainText().split("\n");
	if(ui->chBoxExeptReleases->isChecked()) pathsExept += GetReleases();

	QString res = vfi.ScanFiles(ui->textEditScan->toPlainText().split("\n"),
								ui->lineEditExts->text().split(";"),
								ui->textEditExeptFName->toPlainText().split("\n"),
								pathsExept,
								ui->checkBoxHideIfOne->isChecked());
	if(res != "") QMbw(this, "Errors", "Erros while scan files:\n" + res);

	if(ui->checkBoxHideUpdated->isChecked())
		PrintVectFiles(vectFilesItems::showNeedUpdate);
	else PrintVectFiles(vectFilesItems::showAll);
	QString text;
	if(vfi.countOldFilesTotal == 0) text = "Все файлы обновлены";
	else if(vfi.countOldFilesTotal == 1) text = "Требуется обновление для " + QSn(vfi.countOldFilesTotal) + " файла";
	else text = "Требуются обновления для " + QSn(vfi.countOldFilesTotal) + " файлов";
	ui->labelTextResult->setText(text);
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
			MyQFileDir::ReplaceFilesWithBackup(filesToReplace,fileFind->info, vfi.backupPath);
	}
	else if(messageBox.buttons()[desision]->text() == replaceHimByNewest) // кликнутый заменяем самым новым
	{
		QFileInfo newestModifFI = MyQFileDir::FindNewest(filesFind->GetQFileInfoList());

		// если наш файл новее новейшего - неправльно определён новейший
		if(fileFind->info.lastModified() > newestModifFI.lastModified()) QMb(this,"Ошибка","Ошибка desision == 1. Код ошибки 505");

		if(fileFind->info.lastModified() == newestModifFI.lastModified()) QMb(this,"Замена файлов", "Данный файл является новейшим");
		else
		{
			if(QMessageBox::question(this, "Замена файла", "Заменить файл:\n" + fileFind->info.filePath() + "\n\nфайлом:\n"
									 + newestModifFI.filePath() + "?\n(Резервные копии будут сохранены)",
									 QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
				MyQFileDir::ReplaceFileWithBackup(newestModifFI, fileFind->info, vfi.backupPath);
		}
	}
	else if(messageBox.buttons()[desision]->text() == replaceNothing) ;  // Ничего
	else QMb(this,"Error","Error code 50002");

	on_pushButtonScan_clicked();
}

void IncludeParser::on_pushButtonMassUpdate_clicked()
{
	QStringList values;

	for(auto &f:vfi.vectFiles)
		if(f.needUpdate)
			values += f.name;

	auto chBoxDialogRes = MyQDialogs::CheckBoxDialog(values);
	if(!chBoxDialogRes.accepted) return;

	for(int i=0; i<chBoxDialogRes.checkedTexts.size(); i++)
	{
		for(auto &f:vfi.vectFiles)
			if(chBoxDialogRes.checkedTexts[i] == f.name)
				f.UpdateFiles();
	}

	on_pushButtonScan_clicked();
}
