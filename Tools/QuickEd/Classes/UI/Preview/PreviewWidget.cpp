
#include "PreviewWidget.h"

#include "PreviewModel.h"

#include "EditScreen.h"
#include "SharedData.h"

#include <QLineEdit>
#include <QScreen>
#include <QMessageBox>

#include "Model/PackageHierarchy/ControlNode.h"

#include "QtTools/DavaGLWidget/davaglwidget.h"

using namespace DAVA;

static const int SCALE_PERCENTAGES[] =
{
    10,  25,  50,  75,
	100, 125, 150, 175,
    200, 250, 400, 800,
};

static const int32 DEFAULT_SCALE_PERCENTAGE_INDEX = 4; // 100%
static const char* PERCENTAGE_FORMAT = "%1 %";

PreviewWidget::PreviewWidget(QWidget *parent)
    : QWidget(parent)
    , sharedData(nullptr)
    , model(new PreviewModel(this))
{
    setupUi(this);
    davaGLWidget = new DavaGLWidget();
    frame->layout()->addWidget(davaGLWidget);

    ScopedPtr<UIScreen> davaUIScreen(new DAVA::UIScreen());
    davaUIScreen->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    davaUIScreen->GetBackground()->SetColor(DAVA::Color(0.3f, 0.3f, 0.3f, 1.0f));
    UIScreenManager::Instance()->RegisterScreen(EDIT_SCREEN, davaUIScreen);
    UIScreenManager::Instance()->SetFirst(EDIT_SCREEN);
    UIScreenManager::Instance()->GetScreen()->AddControl(model->GetViewControl());
    
    davaGLWidget->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    model->SetViewControlSize(davaGLWidget->size());

    connect( davaGLWidget, &DavaGLWidget::Resized, this, &PreviewWidget::OnGLWidgetResized );

    // Setup the Scale Combo.
    int scalesCount = COUNT_OF(SCALE_PERCENTAGES);
    for (int i = 0; i < scalesCount; i ++)
    {
        scaleCombo->addItem(QString(PERCENTAGE_FORMAT).arg(SCALE_PERCENTAGES[i]));
    }

    connect(scaleCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PreviewWidget::OnScaleByComboIndex);
    connect(scaleCombo->lineEdit(), &QLineEdit::editingFinished, this, &PreviewWidget::OnScaleByComboText);

    connect(verticalScrollBar, &QScrollBar::valueChanged, this, &PreviewWidget::OnVScrollbarMoved);
    connect(horizontalScrollBar, &QScrollBar::valueChanged, this, &PreviewWidget::OnHScrollbarMoved);

    scaleCombo->setCurrentIndex(DEFAULT_SCALE_PERCENTAGE_INDEX);
    scaleCombo->lineEdit()->setMaxLength(6);
    scaleCombo->setInsertPolicy(QComboBox::NoInsert);
      
    connect(davaGLWidget->GetGLWindow(), &QWindow::screenChanged, this, &PreviewWidget::OnMonitorChanged);

    connect(model, &PreviewModel::CanvasOrViewChanged, this, &PreviewWidget::OnScrollAreaChanged);
    connect(model, &PreviewModel::CanvasPositionChanged, this, &PreviewWidget::OnScrollPositionChanged);
    connect(model, &PreviewModel::CanvasScaleChanged, this, &PreviewWidget::OnCanvasScaleChanged);

    connect(model, &PreviewModel::ControlNodeSelected, this, &PreviewWidget::OnControlNodeSelected);
    connect(model, &PreviewModel::ErrorOccurred, this, &PreviewWidget::OnError);
}

DavaGLWidget *PreviewWidget::GetDavaGLWidget()
{
    return davaGLWidget;
}

void PreviewWidget::OnDocumentChanged(SharedData *data)
{
    if (sharedData == data)
    {
        return;
    }
    sharedData = data;

    UpdateActiveRootControls();
    if (nullptr != sharedData)
    {
        OnScrollAreaChanged(model->GetViewSize(), model->GetScaledCanvasSize());
        OnScrollPositionChanged(model->GetCanvasPosition());
        OnCanvasScaleChanged(model->GetCanvasScale());
        OnMonitorChanged();
        //restore activated controls
        OnDataChanged("activatedControls");
    }
}

void PreviewWidget::OnDataChanged(const QByteArray &role)
{
    if (role == "activeRootControls")
    {
        UpdateActiveRootControls();
    }
    else if (role == "deactivatedControls")
    {
        model->ControlsDeactivated(sharedData->GetData("deactivatedControls").value<QList<ControlNode*> >());
    }
    else if (role == "activatedControls")
    {
        model->ControlsActivated(sharedData->GetData("activatedControls").value<QList<ControlNode*> >());
    }
}

void PreviewWidget::UpdateActiveRootControls()
{
    if (nullptr == sharedData)
    {
        model->SetRootControls(QList<ControlNode*>());
    }
    else
    {
        model->SetRootControls(sharedData->GetData("activeRootControls").value<QList<ControlNode*> >());
    }
}

void PreviewWidget::OnMonitorChanged()
{
    const auto index = scaleCombo->currentIndex();
    scaleCombo->setCurrentIndex(-1);
    scaleCombo->setCurrentIndex(index);
}

void PreviewWidget::OnControlNodeSelected(QList<ControlNode *> selectedNodes)
{
    sharedData->SetData("selectedNodes", QVariant::fromValue(selectedNodes));
}

void PreviewWidget::OnError(const Result &result)
{
    if (result)
    {
        return;
    }
    else
    {
        QMessageBox::warning(qApp->activeWindow(), tr("Error occurred!"), result.errors.join('\n'));
    }
}

void PreviewWidget::OnScaleByZoom(int scaleDelta)
{
    //TODO: implement this method
}

void PreviewWidget::OnScaleByComboIndex(int index)
{   
    if (index < 0 || index >= static_cast<int>( COUNT_OF(SCALE_PERCENTAGES)))
    {
        return;
    }

    auto dpr = static_cast<int>( davaGLWidget->GetGLWindow()->devicePixelRatio() );
    auto scaleValue = SCALE_PERCENTAGES[index] * dpr;
    model->SetCanvasControlScale(scaleValue);
}

void PreviewWidget::OnScaleByComboText()
{
	// Firstly verify whether the value is already set.
	QString curTextValue = scaleCombo->currentText().trimmed();
	int scaleValue = 0;
	if (curTextValue.endsWith(" %"))
	{
		int endCharPos = curTextValue.lastIndexOf(" %");
		QString remainderNumber = curTextValue.left(endCharPos);
		scaleValue = remainderNumber.toInt();
	}
	else
	{
		// Try to parse the value.
		scaleValue = curTextValue.toFloat();
	}
}

void PreviewWidget::OnZoomInRequested()
{
	OnScaleByZoom(10);
}

void PreviewWidget::OnZoomOutRequested()
{
	OnScaleByZoom(-10);
}

void PreviewWidget::OnCanvasScaleChanged(int newScale)
{
    auto dpr = static_cast<int>( davaGLWidget->GetGLWindow()->devicePixelRatio() );
    newScale /= dpr;

    QString newScaleText = QString(PERCENTAGE_FORMAT).arg(newScale);
    if (scaleCombo->currentText() != newScaleText )
    {
        scaleCombo->lineEdit()->blockSignals(true);
        scaleCombo->setEditText(newScaleText);
        scaleCombo->lineEdit()->blockSignals(false);
    }
}

void PreviewWidget::OnGLWidgetResized(int width, int height, int dpr)
{
    Vector2 screenSize(static_cast<float32>(width) * dpr, static_cast<float32>(height) * dpr);

    UIScreenManager::Instance()->GetScreen()->SetSize(screenSize);

    model->SetViewControlSize(QSize(width * dpr, height * dpr));
}

void PreviewWidget::OnVScrollbarMoved(int vPosition)
{
    QPoint canvasPosition = model->GetCanvasPosition();
    canvasPosition.setY(-vPosition);
    model->SetCanvasPosition(canvasPosition);
}

void PreviewWidget::OnHScrollbarMoved(int hPosition)
{
    QPoint canvasPosition = model->GetCanvasPosition();
    canvasPosition.setX(-hPosition);
    model->SetCanvasPosition(canvasPosition);
}

void PreviewWidget::OnScrollPositionChanged(const QPoint &newPosition)
{
    QPoint scrollbarPosition(-newPosition.x(), -newPosition.y());

    if (verticalScrollBar->sliderPosition() != scrollbarPosition.y())
    {
        verticalScrollBar->blockSignals(true);
        verticalScrollBar->setSliderPosition(scrollbarPosition.y());
        verticalScrollBar->blockSignals(false);
    }

    if (horizontalScrollBar->sliderPosition() != scrollbarPosition.x())
    {
        horizontalScrollBar->blockSignals(true);
        horizontalScrollBar->setSliderPosition(scrollbarPosition.x());
        horizontalScrollBar->blockSignals(false);
    }
}

void PreviewWidget::OnScrollAreaChanged(const QSize &viewSize, const QSize &contentSize)
{
    QSize maximumPosition(contentSize - viewSize);

    if (maximumPosition.height() < 0)
    {
        maximumPosition.setHeight(0);
    }

    if (maximumPosition.width() < 0)
    {
        maximumPosition.setWidth(0);
    }

    if (verticalScrollBar->maximum() != maximumPosition.height())
    {
        verticalScrollBar->blockSignals(true);
        verticalScrollBar->setMinimum(0);
        verticalScrollBar->setMaximum(maximumPosition.height());
        verticalScrollBar->setPageStep(viewSize.height());
        verticalScrollBar->blockSignals(false);
    }

    if (horizontalScrollBar->maximum() != maximumPosition.width())
    {
        horizontalScrollBar->blockSignals(true);
        horizontalScrollBar->setMinimum(0);
        horizontalScrollBar->setMaximum(maximumPosition.width());
        horizontalScrollBar->setPageStep(viewSize.width());
        horizontalScrollBar->blockSignals(false);
    }
}