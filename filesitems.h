#ifndef FILESITEMS_H
#define FILESITEMS_H

#include <vector>

#include <QTableWidgetItem>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFileInfo>
#include <QFileInfo>

#include "MyQShortings.h"
#include "MyQFileDir.h"

class FileItem
{
public:
	QFileInfo info;
	QTableWidgetItem *itemFile {nullptr};
	QTableWidgetItem *itemModif {nullptr};
	bool needUpdate = false;
	FileItem(const QFileInfo &info): info {info} {}
};

class FilesItems
{
public:
	QString name;
	QTableWidgetItem *item {nullptr};
	std::vector<FileItem> filesItems;
	bool needUpdate = false;
	QString backupPath;

	FilesItems(const QFileInfo &info, QString backupPath_);
	void UpdateFiles();
	QFileInfoList GetQFileInfoList();
};

class vectFilesItems
{
public:
	std::vector<FilesItems> vectFiles;
	int countOldFilesGroups = 0;
	int countOldFilesTotal = 0;
	QString backupPath;

	inline static QColor colorNew{146,208,80};
	inline static QColor colorOld{255,180,180};
	inline static QString dateFormat { "yyyy.MM.dd hh:mm:ss:zzz" };

	vectFilesItems() = default;
	vectFilesItems(QString backupPath_): backupPath{backupPath_} {}

	int IndexOf(QString name);

	bool Check(const QStringList &chekList, QString val);
	bool CheckExt(const QStringList &chekList, QString val);
	QString ScanFiles(const QStringList &dirsToScan,
					  const QStringList &exts,
					  const QStringList &fnameExept,
					  const QStringList &pathExept,
					  bool hideOneFile);

	enum { showAll, showNeedUpdate };
};

#endif // FILESITEMS_H
