#ifndef __QT_TOOLS_THEMES_H__
#define __QT_TOOLS_THEMES_H__
    
#include <QString>
#include <QStringList>
#include <QPalette>

namespace Themes
{
enum eTheme : int
{
    Light,
    Dark
};
void InitFromQApplication();
QStringList ThemesNames();
void SetCurrentTheme(const QString& theme);
void SetCurrentTheme(eTheme theme);
QString GetCurrentThemeStr();
eTheme GetCurrentTheme();

QColor GetViewLineAlternateColor();
QColor GetChangedPropertyColor();
QColor GetPrototypeColor();
QColor GetStyleSheetNodeColor();
QColor GetRulerWidgetBackgroungColor();
QColor GetHighligtedItemTextColor();
};

#endif // __QT_TOOLS_THEMES_H__
