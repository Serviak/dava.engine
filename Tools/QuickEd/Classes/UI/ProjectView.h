#pragma once
#include <QObject>
#include "UI/mainwindow.h"

class SpritesPacker;
class QComboBox;

class MainWindow::ProjectView : public QObject
{
    Q_OBJECT
public:
    ProjectView(MainWindow* mainWindow);

    void SetProjectPath(const QString& projectPath);
    void SetLanguages(const QStringList& availableLangsCodes, const QString& currentLangCode);
    void SetCurrentLanguage(const QString& currentLang);

    void SetResourceDirectory(const QString& path);
    void SelectFile(const QString& filePath);

    void ExecDialogReloadSprites(SpritesPacker* packer);

    void SetProjectActionsEnabled(bool enable);

    DocumentGroupView* GetDocumentGroupView();

    MainWindow* mainWindow = nullptr;

signals:
    void RtlChanged(bool isRtl);
    void BiDiSupportChanged(bool support);
    void GlobalStyleClassesChanged(const QString& classesStr);
    void CurrentLanguageChanged(const QString& newLangCode);
    void ReloadSprites();
    void FindFileInProject();

private slots:
    void OnRtlChanged(int arg);
    void OnBiDiSupportChanged(int arg);
    void OnGlobalClassesChanged(const QString& str);
    void OnCurrentLanguageChanged(int newLanguageIndex);

private:
    static QString ConvertLangCodeToString(const QString& langCode);

    void InitPluginsToolBar();

    void InitLanguageBox();
    void InitRtlBox();
    void InitBiDiSupportBox();
    void InitGlobalClasses();

    QComboBox* comboboxLanguage = nullptr;
};