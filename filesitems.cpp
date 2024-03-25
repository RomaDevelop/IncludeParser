#include "filesitems.h"

FilesItems::FilesItems(const QFileInfo &info, QString backupPath_):
	name {info.fileName()},
	backupPath {backupPath_}
{
	filesItems.push_back(info);
}

void FilesItems::UpdateFiles()
{
	// нужно найти новейший
	QFileInfoList fiListAll = this->GetQFileInfoList();
	QFileInfo newestModifFI = MQFD::GetNewestFI(fiListAll);

	// нужно найти все не новейшие
	QFileInfoList fiListToReplace;
	for(auto &fi:fiListAll)
		if(fi.lastModified() < newestModifFI.lastModified()) fiListToReplace += fi;

	// нужно заменить с запросом
	MQFD::ReplaceFilesWithBackup(fiListToReplace, newestModifFI, backupPath);
}

QFileInfoList FilesItems::GetQFileInfoList()
{
	QFileInfoList files;
	for(auto &f:this->filesItems)
		files += f.info;
	return files;
}

int vectFilesItems::IndexOf(QString name)
{
	for(uint i=0; i<vectFiles.size(); i++)
		if(vectFiles[i].name == name) return i;
	return -1;
}

bool vectFilesItems::Check(const QStringList &chekList, QString val)
{
	for(auto c:chekList)
	{
		c.replace("\\","/");
		if(c != "" && val.toLower().indexOf(c.toLower()) != -1) return false;
	}
	return true;
}


bool vectFilesItems::CheckExt(const QStringList &chekList, QString val)
{
	for(auto c:chekList)
	{
		c.remove(" ");
		c.remove("*.");
		if(c != "" && val.right(c.length()).toLower() == c) return true;
	}
	return false;
}

void vectFilesItems::ScanFiles(const QStringList &dirsToScan, const QStringList &exts, const QStringList &fnameExept, const QStringList &pathExept, bool hideOneFile)
{
	vectFiles.clear();
	for(auto &d:dirsToScan)
	{
		QDir dir(d);

		//QDirIterator it(dir.path(), /*exts*/QStringList() << "*.dll", QDir::NoFilter, QDirIterator::Subdirectories);
		QDirIterator it(dir.path(), QStringList(), QDir::NoFilter, QDirIterator::Subdirectories);
		while (it.hasNext())
		{
			QFileInfo f(it.next());

			if(f.filePath().indexOf("build-") == -1)
			{
				if(Check(fnameExept,f.fileName()) && Check(pathExept,f.filePath()) && CheckExt(exts,f.fileName()))
				{
					int ind = IndexOf(f.fileName());
					if(ind == -1) vectFiles.push_back({f, backupPath});
					else vectFiles[ind].filesItems.push_back(f);
				}
			}
		}
	}

	if(hideOneFile)
		for(int i=vectFiles.size()-1; i>=0; i--)
			if(vectFiles[i].filesItems.size() < 2)
				vectFiles.erase(vectFiles.begin() + i);

	sort(vectFiles.begin(),vectFiles.end(),[](FilesItems a, FilesItems b){
		return a.name < b.name;
	});
}

void vectFilesItems::PrintVectFiles(QTableWidget *table)
{
	table->setRowCount(0);
	table->clear();
	table->setColumnCount(3);
	table->setColumnWidth(0, table->width()*0.30);
	table->setColumnWidth(1, table->width()*0.50);
	table->setColumnWidth(2, table->width()*0.15);
	//while(1000 >= table->rowCount()) table->insertRow(0);

	int index=0;
	int indexRight = 0;
	for(auto &vf:vectFiles)
	{
		while(index >= table->rowCount()) table->insertRow(index);
		table->setItem(index, 0, new QTableWidgetItem(vf.name));
		vf.item = table->item(index,0);
		indexRight = index;
		index++;
		for(auto &f:vf.filesItems)
		{
			while(indexRight >= table->rowCount()) table->insertRow(indexRight);
			table->setItem(indexRight, 1, new QTableWidgetItem(f.info.filePath()));
			table->setItem(indexRight, 2, new QTableWidgetItem(f.info.lastModified().toString(dateFormat)));
			f.itemFile  = table->item(indexRight,1);
			f.itemModif = table->item(indexRight,2);
			indexRight++;
		}
		if(indexRight > index) index = indexRight;
	}

	// выделение цветом
	for(uint i=0; i<vectFiles.size(); i++)
	{
		int MaxInd = 0;
		for(uint j=0; j<vectFiles[i].filesItems.size(); j++)
		{
			if(vectFiles[i].filesItems[j].info.lastModified() > vectFiles[i].filesItems[MaxInd].info.lastModified()) MaxInd = j;
		}

		for(uint j=0; j<vectFiles[i].filesItems.size(); j++)
		{
			if(vectFiles[i].filesItems[j].info.lastModified() == vectFiles[i].filesItems[MaxInd].info.lastModified())
			{
				vectFiles[i].filesItems[j].itemFile->setBackgroundColor(colorNew);
				vectFiles[i].filesItems[j].itemModif->setBackgroundColor(colorNew);
			}
			else if(vectFiles[i].filesItems[j].info.lastModified() < vectFiles[i].filesItems[MaxInd].info.lastModified())
			{
				vectFiles[i].needUpdate = true;
				vectFiles[i].filesItems[j].itemFile->setBackgroundColor(colorOld);
				vectFiles[i].filesItems[j].itemModif->setBackgroundColor(colorOld);
			}
			else QMb(nullptr,"","Error kod 50001");
		}
	}
}
