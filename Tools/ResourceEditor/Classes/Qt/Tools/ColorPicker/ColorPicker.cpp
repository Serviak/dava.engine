#include "ColorPicker.h"
#include "ui_ColorPicker.h"

#include <QKeyEvent>

#include "AbstractColorPicker.h"
#include "ColorPickerHSV.h"
#include "ColorPickerRGBAM.h"
#include "ColorPreview.h"
#include "EyeDropper.h"

#include "Tools/QtPosSaver/QtPosSaver.h"
#include "Settings/SettingsManager.h"


ColorPicker::ColorPicker(QWidget* parent)
    : AbstractColorPicker(parent)
    , ui(new Ui::ColorPicker())
    , posSaver(new QtPosSaver())
    , confirmed(false)
{
    ui->setupUi(this);
    //posSaver->Attach(this); // Bugs with multiply monitors

    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setFocusPolicy(Qt::ClickFocus);
    setFixedSize(size());

    // Pickers
    RegisterPicker("HSV rectangle", new ColorPickerHSV());

    // Editors
    rgbam = new ColorPickerRGBAM();
    RegisterColorSpace("RGBA M", rgbam);

    // Preview
    connect(this, SIGNAL( changing( const QColor& ) ), ui->preview, SLOT( SetColorNew( const QColor& ) ));
    connect(this, SIGNAL( changed( const QColor& ) ), ui->preview, SLOT( SetColorNew( const QColor& ) ));

    // Dropper
    connect(ui->dropper, SIGNAL( clicked() ), SLOT( OnDropper() ));

    // Color picker
    connect(ui->ok, SIGNAL( clicked() ), SLOT( OnOk() ));
    connect(ui->cancel, SIGNAL( clicked() ), SLOT( close() ));

    // Custom palette
    LoadCustomPalette();
    connect(ui->customPalette, SIGNAL( selected(const QColor&) ), SLOT( OnChanged(const QColor&) ));

    SetColor(Qt::white);
}

ColorPicker::~ColorPicker()
{
}

bool ColorPicker::Exec(const QString& title)
{
    const Qt::WindowFlags f = windowFlags();
    const Qt::WindowModality m = windowModality();
    setWindowFlags(f | Qt::Dialog);
    setWindowModality(Qt::ApplicationModal);
    setWindowOpacity(1.0);
    if (!title.isEmpty())
    {
        setWindowTitle(title);
    }

    show();
    modalLoop.exec();

    setWindowFlags(f);
    setWindowModality(m);

    return confirmed;
}

double ColorPicker::GetMultiplierValue() const
{
    if (rgbam)
    {
        return rgbam->GetMultiplierValue();
    }

    return 0.0;
}

void ColorPicker::SetMultiplierValue(double val)
{
    if (rgbam)
    {
        rgbam->SetMultiplierValue(val);
    }
}

void ColorPicker::RegisterPicker(QString const& key, AbstractColorPicker* picker)
{
    delete pickers[key];
    pickers[key] = picker;

    ui->pickerCombo->addItem(key, key);
    ui->pickerStack->addWidget(picker);
    ConnectPicker(picker);
}

void ColorPicker::RegisterColorSpace(const QString& key, AbstractColorPicker* picker)
{
    delete colorSpaces[key];
    colorSpaces[key] = picker;

    ui->colorSpaceCombo->addItem(key, key);
    ui->colorSpaceStack->addWidget(picker);
    ConnectPicker(picker);
}

void ColorPicker::SetColorInternal(const QColor& c)
{
    UpdateControls(c);
    oldColor = c;
    ui->preview->SetColorOld(c);
    ui->preview->SetColorNew(c);
}

void ColorPicker::OnChanging(const QColor& c)
{
    AbstractColorPicker* source = qobject_cast<AbstractColorPicker *>(sender());
    UpdateControls(c, source);
    emit changing(c);
}

void ColorPicker::OnChanged(const QColor& c)
{
    AbstractColorPicker* source = qobject_cast<AbstractColorPicker *>(sender());
    UpdateControls(c, source);
    emit changed(c);
}

void ColorPicker::OnDropperChanged(const QColor& c)
{
    QColor normalized(c);
    normalized.setAlphaF(GetColor().alphaF());
    UpdateControls(normalized);
    ui->preview->SetColorNew(normalized);
}

void ColorPicker::OnDropper()
{
    dropper = new EyeDropper(this);
    connect(dropper, SIGNAL( picked( const QColor& ) ), SLOT( OnDropperChanged( const QColor& ) ));
    connect(dropper, SIGNAL( picked( const QColor& ) ), SLOT( show() ));
    connect(dropper, SIGNAL( canceled() ), SLOT( show() ));
    const qreal opacity = windowOpacity();
    setWindowOpacity(0.0);
    hide();
    dropper->Exec();
    setWindowOpacity(opacity);
}

void ColorPicker::OnOk()
{
    confirmed = true;
    emit changed(GetColor());
    close();
}

void ColorPicker::UpdateControls(const QColor& c, AbstractColorPicker* source)
{
    for (auto it = pickers.begin(); it != pickers.end(); ++it)
    {
        AbstractColorPicker* recv = it.value();
        if (recv && recv != source)
        {
            recv->SetColor(c);
        }
    }
    for (auto it = colorSpaces.begin(); it != colorSpaces.end(); ++it)
    {
        AbstractColorPicker* recv = it.value();
        if (recv && recv != source)
        {
            recv->SetColor(c);
        }
    }

    ui->preview->SetColorNew(c);
    color = c;
}

void ColorPicker::ConnectPicker(AbstractColorPicker* picker)
{
    connect(picker, SIGNAL( begin() ), SIGNAL( begin() ));
    connect(picker, SIGNAL( changing( const QColor& ) ), SLOT( OnChanging( const QColor& ) ));
    connect(picker, SIGNAL( changed( const QColor& ) ), SLOT( OnChanged( const QColor& ) ));
    connect(picker, SIGNAL( canceled() ), SIGNAL( canceled() ));
}

void ColorPicker::closeEvent(QCloseEvent* e)
{
    if (modalLoop.isRunning())
    {
        modalLoop.quit();
    }
    SaveCustomPalette();

    QWidget::closeEvent(e);
}

void ColorPicker::keyPressEvent(QKeyEvent* e)
{
    switch(e->key())
    {
    case Qt::Key_Escape:
        close();
        break;
    case Qt::Key_Enter:
        OnOk();
        break;
    }

    return QWidget::keyPressEvent(e);
}

void ColorPicker::LoadCustomPalette()
{
    DAVA::VariantType v = SettingsManager::Instance()->GetValue(Settings::Internal_CustomPalette);
    const DAVA::int32 vSize = v.AsByteArraySize();
    const DAVA::int32 n = vSize / sizeof(DAVA::int32);
    const DAVA::uint32 *a = (DAVA::uint32 *)v.AsByteArray();

    CustomPalette::Colors colors(n);
    for (int i = 0; i < n; i++)
    {
        colors[i] = QColor::fromRgba(a[i]);
    }

    ui->customPalette->SetColors(colors);
}

void ColorPicker::SaveCustomPalette()
{
    const CustomPalette::Colors& colors = ui->customPalette->GetColors();
    const DAVA::int32 n = colors.size();
    QVector<DAVA::uint32> a(n);
    for (int i = 0; i < n; i++)
    {
        a[i] = colors[i].rgba();
    }

    const DAVA::VariantType v((DAVA::uint8 *)a.data(), n*sizeof(DAVA::uint32));
    SettingsManager::Instance()->SetValue(Settings::Internal_CustomPalette, v);
}
