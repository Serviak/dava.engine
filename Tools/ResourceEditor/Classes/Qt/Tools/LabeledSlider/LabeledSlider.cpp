#include "LabeledSlider.h"

#include "DAVAEngine.h"

#include <QSlider>
#include <QLabel>
#include <QGridLayout>

LabeledSlider::LabeledSlider(QWidget *parent /*= 0*/)
	: QWidget(parent)
{
    InitUI();
    
    connect(slider, SIGNAL(valueChanged(int)), SLOT(ValueChanged(int)));
    connect(slider, SIGNAL(rangeChanged(int, int)), SLOT(RangeChanged(int, int)));
    
    slider->setRange(0, 1);
    slider->setValue(1);
}


LabeledSlider::~LabeledSlider()
{

}

void LabeledSlider::InitUI()
{
    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    
    slider = new QSlider(this);
    slider->setObjectName(QString::fromUtf8("slider"));
    
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(slider->sizePolicy().hasHeightForWidth());
    slider->setSizePolicy(sizePolicy);
    slider->setMinimumSize(QSize(120, 20));
    slider->setOrientation(Qt::Horizontal);
    
    gridLayout->addWidget(slider, 0, 0, 1, 3);
    
    minText = new QLabel(this);
    minText->setObjectName(QString::fromUtf8("minText"));
    QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Minimum);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(minText->sizePolicy().hasHeightForWidth());
    minText->setSizePolicy(sizePolicy1);
    minText->setMinimumSize(QSize(40, 20));
    
    gridLayout->addWidget(minText, 1, 0, 1, 1);
    
    valueText = new QLabel(this);
    valueText->setObjectName(QString::fromUtf8("valueText"));
    sizePolicy.setHeightForWidth(valueText->sizePolicy().hasHeightForWidth());
    valueText->setSizePolicy(sizePolicy);
    valueText->setMinimumSize(QSize(40, 20));
    valueText->setAlignment(Qt::AlignCenter);
    
    gridLayout->addWidget(valueText, 1, 1, 1, 1);
    
    maxText = new QLabel(this);
    maxText->setObjectName(QString::fromUtf8("maxText"));
    sizePolicy1.setHeightForWidth(maxText->sizePolicy().hasHeightForWidth());
    maxText->setSizePolicy(sizePolicy1);
    maxText->setMinimumSize(QSize(40, 20));
    maxText->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    
    gridLayout->addWidget(maxText, 1, 2, 1, 1);
}

int LabeledSlider::value() const
{
    return slider->value();
}

int LabeledSlider::minimum() const
{
    return slider->minimum();
}

void LabeledSlider::setMinimum(int min)
{
    slider->setMinimum(min);
}

int LabeledSlider::maximum() const
{
    return slider->maximum();
}

void LabeledSlider::setMaximum(int max)
{
    slider->setMaximum(max);
}

void LabeledSlider::setRange(int min, int max)
{
    slider->setRange(min, max);
}

void LabeledSlider::setValue(int val)
{
    bool wasBlocked = slider->blockSignals(true);

    slider->setValue(val);

    slider->blockSignals(wasBlocked);
}

void LabeledSlider::ValueChanged(int value)
{
    valueText->setText(QString::fromAscii(DAVA::Format("%d", value)));
    
    emit valueChanged(value);
}

void LabeledSlider::RangeChanged(int min, int max)
{
    minText->setText(QString::fromAscii(DAVA::Format("%d", min)));
    maxText->setText(QString::fromAscii(DAVA::Format("%d", max)));
}
