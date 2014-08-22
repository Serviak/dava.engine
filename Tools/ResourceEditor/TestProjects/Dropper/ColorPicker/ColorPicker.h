#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QWidget>
#include <QScopedPointer>
#include <QMap>
#include <QPointer>

namespace Ui {class ColorPicker;};

#include "AbstractColorPicker.h"


class EyeDropper;

class ColorPicker
    : public AbstractColorPicker
{
    Q_OBJECT

private:
    typedef QMap< QString, QPointer<AbstractColorPicker> > PickerMap;

public:
    explicit ColorPicker(QWidget *parent = 0);
    ~ColorPicker();

protected:
    void RegisterPicker( const QString& key, AbstractColorPicker *picker );
    void RegisterColorSpace( const QString& key, AbstractColorPicker *picker );

    void SetColorInternal( const QColor& c ) override;

private slots:
    void OnChanging( const QColor& c );
    void OnChanged( const QColor& c );

private:
    void UpdateControls( const QColor& c, AbstractColorPicker *source = NULL );
    void ConnectPicker( AbstractColorPicker *picker );

    QScopedPointer<Ui::ColorPicker> ui;
    QScopedPointer<EyeDropper> dropper;
    PickerMap pickers;
    PickerMap colorSpaces;
};

#endif // COLORPICKER_H
