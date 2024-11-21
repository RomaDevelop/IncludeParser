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
	QFileInfo newestModifFI = MyQFileDir::FindNewest(fiListAll);

	// нужно найти все не новейшие
	QFileInfoList fiListToReplace;
	for(auto &fi:fiListAll)
		if(fi.lastModified() < newestModifFI.lastModified()) fiListToReplace += fi;

	// нужно заменить с запросом
	MyQFileDir::ReplaceFilesWithBackup(fiListToReplace, newestModifFI, backupPath);
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
		if(c != "" && val.toLower().contains(c.toLower()))
			return false;
	}
	return true;
}


bool vectFilesItems::CheckExt(const QStringList &chekList, QString suffix)
{
	for(auto c:chekList)
	{
		c.remove(" ");
		c.remove("*.");
		if(c != "" && suffix == c) return true;
	}
	return false;
}

QString vectFilesItems::ScanFiles(const QStringList &dirsToScan,
								  const QStringList &exts,
								  const QStringList &fnameExept,
								  const QStringList &pathExept,
								  bool hideOneFile)
{
	QString error;
	vectFiles.clear();
	for(auto &d:dirsToScan)
	{
		QDir dir(d);
		if(dir.exists())
		{
			QDirIterator it(dir.path(), QStringList(), QDir::Files, QDirIterator::Subdirectories);
			while (it.hasNext())
			{
				QFileInfo file(it.next());
				if(Check(fnameExept,file.fileName()) && Check(pathExept,file.path()) && CheckExt(exts,file.suffix()))
				{
					int ind = IndexOf(file.fileName());
					if(ind == -1) vectFiles.push_back({file, backupPath});
					else vectFiles[ind].filesItems.push_back(file);
				}
			}
		}
		else error += "vectFilesItems::ScanFiles dir [" + d + "] doesn't exists\n";
	}

	if(hideOneFile)
		for(int i=vectFiles.size()-1; i>=0; i--)
			if(vectFiles[i].filesItems.size() < 2)
				vectFiles.erase(vectFiles.begin() + i);

	sort(vectFiles.begin(),vectFiles.end(),[](FilesItems a, FilesItems b){
		return a.name < b.name;
	});

	// определяем нуждающиеся в обновлении
	for(uint i=0; i<vectFiles.size(); i++)
	{
		QDateTime lastestModified;  // определяем последнее обновление
		for(uint j=0; j<vectFiles[i].filesItems.size(); j++)
		{
			if(vectFiles[i].filesItems[j].info.lastModified() > lastestModified)
				lastestModified = vectFiles[i].filesItems[j].info.lastModified();
		}

		vectFiles[i].needUpdate = false;
		for(uint j=0; j<vectFiles[i].filesItems.size(); j++)
		{
			if(vectFiles[i].filesItems[j].info.lastModified() == lastestModified)
			{
				vectFiles[i].filesItems[j].needUpdate = false;
			}
			else if(vectFiles[i].filesItems[j].info.lastModified() < lastestModified)
			{
				vectFiles[i].needUpdate = true;
				vectFiles[i].filesItems[j].needUpdate = true;
				countOldFilesTotal++;
			}
			else
			{
				QMb(nullptr,"","Error kod 50001");
				break;
			}
		}
	}

	// считае количество нуждающихся в одновлении файлов
	countOldFilesTotal = 0;
	countOldFilesGroups = 0;
	for(uint i=0; i<vectFiles.size(); i++)
	{
		if(vectFiles[i].needUpdate)
			countOldFilesGroups++;

		for(uint j=0; j<vectFiles[i].filesItems.size(); j++)
		{
			if(vectFiles[i].filesItems[j].needUpdate)
				countOldFilesTotal++;
		}
	}

	return error;
}

void vectFilesItems::PrintVectFiles(QTableWidget *table, int showCode)
{
	table->setRowCount(0);
	table->clear();
	table->setColumnCount(3);
	table->setColumnWidth(0, table->width()*0.30);
	table->setColumnWidth(1, table->width()*0.50);
	table->setColumnWidth(2, table->width()*0.15);

	int index=0;
	int indexRight = 0;
	for(auto &vf:vectFiles)
	{
		if(showCode == showAll) {}
		else if(showCode == showNeedUpdate) { if(!vf.needUpdate) continue; }
		else { QMbc(nullptr, "wrong showCode", "wrong showCode"); }

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
			if(f.needUpdate) { f.itemFile->setBackgroundColor(colorOld); f.itemModif->setBackgroundColor(colorOld); }
			else { f.itemFile->setBackgroundColor(colorNew); f.itemModif->setBackgroundColor(colorNew); }
			indexRight++;
		}
		if(indexRight > index) index = indexRight;
	}
}
