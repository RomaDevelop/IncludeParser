#include "includeparser.h"
#include "ui_includeparser.h"

#include <algorithm>
#include <vector>
using namespace std;

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QDirIterator>
#include <QDateTime>
#include <QMessageBox>
#include <QCheckBox>

#include "MyQShortings.h"
#include "MyQStr.h"

class Files
{
public:
	QFileInfo leftFile;
	vector<pair<QTableWidgetItem*,QFileInfo*>> items_files;
	QFileInfoList rightFiles;
};

class FileItem
{
public:
	QFileInfo info;
	QTableWidgetItem *itemFile;
	QTableWidgetItem *itemModif;
};

class FilesNew
{
public:
	QString name;
	QTableWidgetItem *item;
	FileItem fileItem;
};

vector<Files> vectFiles;

Ui::IncludeParser *uiObj;
QString dateFormat = "yyyy.MM.dd hh:mm:ss:zzz";
QString backupPath;

IncludeParser::IncludeParser(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::IncludeParser)
{
	ui->setupUi(this);
	uiObj = ui;
	this->move(30,30);

	toSave.push_back(ui->lineEditIncludePath);

	QDir pathBackup(QFileInfo(QCoreApplication::applicationFilePath()).path() + "/files");
	if(!pathBackup.exists()) pathBackup.mkdir(pathBackup.path());
	pathBackup.setPath(pathBackup.path() + "/backup");
	if(!pathBackup.exists()) pathBackup.mkdir(pathBackup.path());
	if(!pathBackup.exists()) { QMessageBox::information(this, "Ошибка", "Ошибка создания директории для файла резервного копирования. Резервные копии не будут сохраняться!"); return; }
	backupPath = pathBackup.path();

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
				QString setting = MyStr::GetElemetFromStrings(Settings,spl,i);
				QString name = MyStr::GetElemetFromStrings(setting,"[s;]",1);
				QString className = MyStr::GetElemetFromStrings(setting,"[s;]",2);
				QString value = MyStr::GetElemetFromStrings(setting,"[s;]",3);

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
	if(!pathSave.exists()) pathSave.mkdir(pathSave.path());
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

int GetIndex(QString text)
{
	for(int i=0; i<uiObj->tableWidget->rowCount(); i++)
	{
		if(uiObj->tableWidget->item(i,0)->text() == text)
		{
			return i;
		}
	}
	return -1;
}

void PrintVectFiles(QTableWidget *table)
{
	table->clear();
	table->setColumnCount(4);
	table->setColumnWidth(0, table->width()*0.20);
	table->setColumnWidth(1, table->width()*0.10);
	table->setColumnWidth(2, table->width()*0.50);
	table->setColumnWidth(3, table->width()*0.10);
	//while(1000 >= table->rowCount()) table->insertRow(0);

	int index=0;
	int indexRight = 0;
	for(auto &vf:vectFiles)
	{
		while(index >= table->rowCount()) table->insertRow(index);
		table->setItem(index, 0, new QTableWidgetItem(vf.leftFile.fileName()));
		table->setItem(index, 1, new QTableWidgetItem(vf.leftFile.lastModified().toString(dateFormat)));
		vf.items_files.push_back({table->item(index,1), &vf.leftFile});
		indexRight = index;
		index++;
		for(auto &f:vf.rightFiles)
		{
			while(indexRight >= table->rowCount()) table->insertRow(indexRight);
			table->setItem(indexRight, 2, new QTableWidgetItem(f.filePath()));
			table->setItem(indexRight, 3, new QTableWidgetItem(f.lastModified().toString(dateFormat)));
			vf.items_files.push_back({table->item(indexRight,3), &f});
			indexRight++;
		}
		if(indexRight > index) index = indexRight;
	}
}

void IncludeParser::on_pushButton_clicked()
{
	ui->lineEditIncludePath->setText(ui->lineEditIncludePath->text().replace("\\","/"));

	QDir dirLeft(ui->lineEditIncludePath->text());

	QFileInfoList dirContentLeft = dirLeft.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
	QFileInfoList dirContentRight;

	QString allFiles;
	vectFiles.clear();
	for(auto &f:dirContentLeft)
	{
		Files files;
		files.leftFile = f;
		vectFiles.push_back(files);
		allFiles += f.fileName();
	}

	QDir dirRight = dirLeft;
	dirRight.cdUp();
	QDirIterator it(dirRight.path(), QStringList() << "*.h", QDir::NoFilter, QDirIterator::Subdirectories);
	while (it.hasNext())
	{
		QFileInfo f(it.next());

		if(f.filePath().indexOf("build-") == -1 &&
				allFiles.indexOf(f.fileName()) != -1 &&
				f.filePath().indexOf(dirLeft.path()) == -1)
		{
			dirContentRight.append(f);
		}
	}

	for(auto &f:dirContentRight)
	{
		bool find = false;
		for(auto &vf:vectFiles)
		{
			if(vf.leftFile.fileName() == f.fileName())
			{
				find = true;
				vf.rightFiles.push_back(f);
			}
		}
		if(!find) qdbg << "ошибка работы, не дожно быть таких файлов ("+ f.filePath() +") в списке dirContentRight";
	}

	PrintVectFiles(ui->tableWidget);
	for(auto &vf:vectFiles)
	{
		QString MaxVal = vf.items_files[0].first->text();
		for(auto i:vf.items_files)
			if(MaxVal < i.first->text()) MaxVal = i.first->text();

		for(auto i:vf.items_files)
			if(i.first->text() == MaxVal) i.first->setBackgroundColor(QColor(146,208,80));
			else i.first->setBackgroundColor(QColor(255,180,180));
	}
}

void replaceFile(QFileInfo &src, QFileInfo &dst)
{
	QFile fileToReplace(dst.filePath());
	QString backupFile = backupPath + "/" + QDateTime::currentDateTime().toString(dateFormat).replace(':','.') + " " + dst.fileName();
	if(!fileToReplace.copy(backupFile)) QMb(nullptr,"Ошибка","Не удалось создать backup-файл" + backupFile);
	else
	{
		if(!fileToReplace.remove()) QMb(nullptr,"Ошибка","Не удалось удалить файл " + fileToReplace.fileName());
		if(!QFile::copy(src.filePath(),dst.filePath())) QMb(nullptr,"Ошибка","Не удалось создать файл " + dst.fileName());
	}
}

void IncludeParser::on_tableWidget_cellDoubleClicked(int row, int column)
{
	ui->lineEdit->setText("row="+QSn(row)+" col="+QSn(column));
	QTableWidgetItem *item = ui->tableWidget->item(row, column);
	QTableWidgetItem *findedFileItem {nullptr};
	QFileInfo *findedFileInfo {nullptr};
	Files *filesFind {nullptr};

	for(uint i=0; i<vectFiles.size(); i++)
	{
		for(uint j=0; j<vectFiles[i].items_files.size(); j++)
			if(vectFiles[i].items_files[j].first == item)
			{
				findedFileItem = vectFiles[i].items_files[j].first;
				findedFileInfo = vectFiles[i].items_files[j].second;
				filesFind = &vectFiles[i];
			}
	}

	if(!findedFileItem || !findedFileInfo) return;

	QMessageBox messageBox(QMessageBox::Question, "Замена файлов", "Вы кликнули на файл " + findedFileInfo->filePath() + ".\n\nЧто нужно сделать?");
	messageBox.addButton("Заменить им все аналогичные",QMessageBox::YesRole);
	messageBox.addButton("Заменить его новейшим",QMessageBox::YesRole);
	messageBox.addButton("Ничего",QMessageBox::NoRole);
	int desision =  messageBox.exec();

	if(desision == 0) // вариант 0 - кликнутый копируем на место всех
	{
		QFileInfoList files;
		files += filesFind->leftFile;
		files += filesFind->rightFiles;
		QFileInfoList filesToReplace;

		// берем все файлы кроме нашаего
		for(auto &f:files)
			if(f.filePath() != findedFileInfo->filePath())
				filesToReplace += f;

		// формруем QString с именами файлов, которые будут заменены
		QString filestpReplaceStr;
		for(auto &f:filesToReplace)
		{
			QString fileStrPlus = f.filePath();
			filestpReplaceStr += fileStrPlus + "    (" + f.lastModified().toString(dateFormat) + ")\n";
		}

		if(!filesToReplace.empty())
		{
			QString replaceFileStr = findedFileInfo->filePath() + "    (" + findedFileItem->text() + ")";
			if(QMessageBox::question(this, "Замена файлов", "Заменить файлы:\n" + filestpReplaceStr + "\n\nфайлом:\n" + replaceFileStr + "?\n(Резервные копии будут сохранены)",
									 QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
			{
				for(auto &f:filesToReplace)
				{
					replaceFile(*findedFileInfo,f);
				}
			}
		}
	}
	if(desision == 1) // вариант 1 - кликнутый заменяем самым новым
	{
		QFileInfoList files;
		files += filesFind->leftFile;
		files += filesFind->rightFiles;

		QFileInfo newestLastModifFI = files[0];
		for(auto &f:files)
			if(f.lastModified() > newestLastModifFI.lastModified())
				newestLastModifFI = f;

		// если наш файл новее новейшего - неправльно определён новейший
		if(findedFileInfo->lastModified() > newestLastModifFI.lastModified()) QMb(this,"Ошибка","Ошибка desision == 1. Код ошибки 505");

		if(findedFileInfo->lastModified() == newestLastModifFI.lastModified()) QMb(this,"Замена файлов", "Данный файл является новейшим");
		else
		{
			if(QMessageBox::question(this, "Замена файла", "Заменить файл:\n" + findedFileInfo->filePath() + "\n\nфайлом:\n" + newestLastModifFI.filePath() + "?\n(Резервные копии будут сохранены)",
									 QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
			{
				replaceFile(newestLastModifFI,*findedFileInfo);
			}
		}
	}
	//if(desision == 2)  // Ничего

	on_pushButton_clicked();
}
