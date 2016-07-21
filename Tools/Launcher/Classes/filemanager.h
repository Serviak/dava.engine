#pragma once

#include <QString>

namespace FileManager
{
QString GetDocumentsDirectory();
QString GetBaseAppsDirectory();
QString GetTempDirectory();
QString GetLauncherDirectory();
QString GetSelfUpdateTempDirectory();
QString GetTempDownloadFilePath();

bool CreateFileAndWriteData(const QString& filePath, const QByteArray& data);
bool DeleteDirectory(const QString& path);

//this function move all files and folder except folders, which created by Launcher
bool MoveLauncherRecursively(const QString& pathOut, const QString& pathIn);

void MakeDirectory(const QString& path);

QString GetApplicationDirectory(const QString& branchID, const QString& appID);
QString GetApplicationDirectoryOld(const QString& branchID, const QString& appID);

QString GetBranchDirectory(const QString& branchID);
QString GetBranchDirectoryOld(const QString& branchID);
};
