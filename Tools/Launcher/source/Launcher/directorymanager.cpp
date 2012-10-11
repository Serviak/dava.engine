#include "directorymanager.h"
#include <QtCore>

DirectoryManager* DirectoryManager::m_spInstance = NULL;

#define DOWNLOAD_DIR "/Downloads/"
#define STABLE_DIR "/Stable/"
#define DEVELOPMENT_DIR "/Development/"
#define DEPENDENCIES_DIR "/Dependencies/"

DirectoryManager* DirectoryManager::GetInstance() {
    if (!m_spInstance)
        m_spInstance = new DirectoryManager();
    return m_spInstance;
}

DirectoryManager::DirectoryManager(QObject *parent) :
    QObject(parent) {
}

void DirectoryManager::Init() {
    InitBaseDir();

    QDir().mkdir(GetDownloadDir());
    QDir().mkdir(GetStableDir());
    QDir().mkdir(GetDevelopment());
    QDir().mkdir(GetDependencies());
}

void DirectoryManager::InitBaseDir() {
    m_runPath = qApp->applicationFilePath();
    m_appDir = qApp->applicationDirPath();
#ifdef Q_OS_DARWIN
    m_appDir.replace("/Contents/MacOS", "");
#endif

    m_configDir = m_appDir;

    int nPos = -1;
    do {
        int pos = m_appDir.indexOf("/", nPos + 1);
        if (pos != -1)
            nPos = pos;
        else
            break;
    }while (true);
    if (nPos != -1)
        m_appDir.chop(m_appDir.size() - nPos);

#ifdef Q_OS_DARWIN
     m_configDir = m_appDir;
#endif
}

QString DirectoryManager::GetBaseDirectory() const {
    return m_appDir;
}

QString DirectoryManager::GetAppDirectory() const {
    QString  appDir = qApp->applicationDirPath();
#if defined(Q_OS_DARWIN) || defined(Q_OS_DARWIN64)
    appDir.replace("/Contents/MacOS", "");
#endif
    return appDir;
}

QString DirectoryManager::GetDownloadDir() const {
    QString baseDir = GetBaseDirectory();
    baseDir.append(DOWNLOAD_DIR);
    return baseDir;
}

QString DirectoryManager::GetStableDir() const {
    QString baseDir = GetBaseDirectory();
    baseDir.append(STABLE_DIR);
    return baseDir;
}

QString DirectoryManager::GetDevelopment() const {
    QString baseDir = GetBaseDirectory();
    baseDir.append(DEVELOPMENT_DIR);
    return baseDir;
}

QString DirectoryManager::GetDependencies() const {
    QString baseDir = GetBaseDirectory();
    baseDir.append(DEPENDENCIES_DIR);
    return baseDir;
}

bool DirectoryManager::DeleteDir(const QString& path) {
    bool result = true;
    QDir aDir(path);
    if (aDir.exists())//QDir::NoDotAndDotDot
    {
        QFileInfoList entries = aDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden);
        int count = entries.size();
        for (int idx = 0; idx < count; idx++)
        {
            QFileInfo entryInfo = entries[idx];
            QString path = entryInfo.absoluteFilePath();
            if (entryInfo.isDir())
            {
                result = DeleteDir(path);
            }
            else
            {
                QFile file(path);
                if (!file.remove())
                    result = false;
            }
        }
        if (!aDir.rmdir(aDir.absolutePath()))
            result = false;
    }
    return result;
}

bool DirectoryManager::CopyAllFromDir(const QString& srcPath, const QString& destPath) {
    bool result = true;
    QDir aDir(srcPath);
    if (aDir.exists())//QDir::NoDotAndDotDot
    {
        QFileInfoList entries = aDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden);
        int count = entries.size();
        for (int idx = 0; idx < count; idx++)
        {
            QFileInfo entryInfo = entries[idx];

            if (entryInfo.isSymLink())
                continue;

            QString path = entryInfo.absoluteFilePath();
            path = path.replace(srcPath, "", Qt::CaseInsensitive);
            if (entryInfo.isDir())
            {
                if (!QDir(destPath + path).exists() && !QDir().mkdir(destPath + path))
                {
                    result = false;
                    continue;
                }
                CopyAllFromDir(srcPath + path, destPath + path);
            }
            else
            {
                if (!QFile().copy(srcPath + path, destPath + path))
                    result = false;
            }
        }
        //if (!aDir.rmdir(aDir.absolutePath()))
        //    result = false;
    }
    return result;
}

QStringList DirectoryManager::GetDirectoryStructure(const QString& path) {
    QStringList list;

    QDir aDir(path);
    if (aDir.exists())//QDir::NoDotAndDotDot
    {
        QFileInfoList entries = aDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden);
        int count = entries.size();
        for (int idx = 0; idx < count; idx++)
        {
            QFileInfo entryInfo = entries[idx];
            list.append(entryInfo.fileName());
        }
    }

    return list;
}

QString DirectoryManager::GetConfigDir() const {
    return m_configDir;
}
