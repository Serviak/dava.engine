/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "Themes.h"
#include <QtGlobal>
#include <QStyle>
#include <QApplication>
#include <QStyleFactory>
#include <QSettings>

namespace Themes_namespace
{
const QString themeSettingsGroup = "QtTools/Themes";
const QString themeSettingsKey = "ThemeName";
}


namespace Themes
{
QPalette defaultPalette;
QString defaultStyleSheet;
eTheme currentTheme;
bool themesInitialized = false;
QStringList themesNames = {"classic", "dark"};
    
void SetupClassicTheme();
void SetupDarkTheme();
    
void InitFromQApplication()
{
    themesInitialized = true;
    defaultStyleSheet = qApp->styleSheet();
    defaultPalette = qApp->palette();
    qAddPostRoutine([](){
        QSettings settings(QApplication::organizationName(), QApplication::applicationName());
        settings.beginGroup(Themes_namespace::themeSettingsGroup);
        settings.setValue(Themes_namespace::themeSettingsKey, static_cast<int>(currentTheme));
        settings.endGroup();
        settings.sync();
    });
    QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    settings.beginGroup(Themes_namespace::themeSettingsGroup);
    auto value = settings.value(Themes_namespace::themeSettingsKey);
    settings.endGroup();
    if(value.canConvert<int>())
    {
        currentTheme = static_cast<eTheme>(value.value<int>());
    }
    else
    {
        currentTheme = Classic;
    }
    SetCurrentTheme(currentTheme);
}

QStringList ThemesNames()
{
    return themesNames;
}
    
void SetCurrentTheme(const QString& theme)
{
    if(!themesNames.contains(theme))
    {
        qWarning("Invalid theme passed to SetTheme");
        return;
    }
    int index = themesNames.indexOf(theme);
    SetCurrentTheme(static_cast<eTheme>(index));
}

void SetCurrentTheme(eTheme theme)
{
    if (!themesInitialized)
    {
        qWarning("ThemesFactiry uninitialized");
        return;
    }
    currentTheme = theme;
    switch (theme)
    {
        case Classic:
            SetupClassicTheme();
            break;
        case Dark:
            SetupDarkTheme();
            break;
    }
}
    
void SetupClassicTheme()
{
#ifdef Q_OS_MAC
    QString styleName = "macintosh";
#else
    QString styleName = "windowsVista";
#endif //Q_OS_MAC
    qApp->setStyle(QStyleFactory::create(styleName));
    qApp->setPalette(defaultPalette);
    qApp->setStyleSheet(defaultStyleSheet);
}
    
void SetupDarkTheme()
{
    qApp->setStyle(QStyleFactory::create("Fusion"));
    
    QPalette darkPalette;
    QColor textColor(0xf2, 0xf2, 0xf2);
    darkPalette.setColor(QPalette::Window, QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText, textColor);
    darkPalette.setColor(QPalette::Base, QColor(25,25,25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    darkPalette.setColor(QPalette::ToolTipBase, textColor);
    darkPalette.setColor(QPalette::ToolTipText, textColor);
    darkPalette.setColor(QPalette::Text, textColor);
    darkPalette.setColor(QPalette::Button, QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText, textColor);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    
    darkPalette.setColor(QPalette::Highlight, QColor(0x37, 0x63, 0xAD));
    darkPalette.setColor(QPalette::HighlightedText, QColor(Qt::white));
    
    qApp->setPalette(darkPalette);
    
    qApp->setStyleSheet("QToolTip { color: #f2f2f2; background-color: #2a82da; border: 1px solid white; }");
}


const QString& GetCurrentThemeStr()
{
    return themesNames.at(currentTheme);
}
    
eTheme GetCurrentTheme()
{
    return currentTheme;
}
    
QColor GetViewLineAliternateColor()
{
    return QColor(0x3f, 0x3f, 0x46);
}

};
