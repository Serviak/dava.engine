#include "importdialogfilewidget.h"
#include "ui_importdialogfilewidget.h"
#include "IconHelper.h"

#define EVEN_COLOR QColor(0xFF, 0xFF, 0xFF)
#define ODD_COLOR QColor(0xEE, 0xEE, 0xEE)
#define ACTIVE_FILE_COLOR QColor(0, 0, 0)
#define IGNORED_FILE_COLOR QColor(0xA0, 0xA0, 0xA0)

ImportDialogFileWidget::ImportDialogFileWidget(uint32 id, QWidget *parent)
:	QWidget(parent)
,	ui(new Ui::ImportDialogFileWidget)
,	id(id)
,	neverShowSizeWidget(false)
,	showUpperIcons(false)
,	activeAction(ImportDialog::ACTION_IGNORE)
{
	ui->setupUi(this);
	ui->resetToParentButton->setIcon(QIcon(IconHelper::GetIgnoreIconPath()));
	ui->upperIgnoreIcon->setPixmap(QIcon(IconHelper::GetIgnoreIconPath()).pixmap(16, 16));
	ui->upperScreenIcon->setPixmap(QIcon(IconHelper::GetScreenIconPath()).pixmap(16, 16));
	ui->upperAggregatorIcon->setPixmap(QIcon(IconHelper::GetAggregatorIconPath()).pixmap(16, 16));

	UpdateState(ImportDialog::ACTION_IGNORE);
	SetFilename("");

	ui->actionButtons->setId(ui->ignoreRadioButton, ImportDialog::ACTION_IGNORE);
	ui->actionButtons->setId(ui->screenRadioButton, ImportDialog::ACTION_SCREEN);
	ui->actionButtons->setId(ui->aggregatorRadioButton, ImportDialog::ACTION_AGGREGATOR);

	connect(ui->actionButtons, SIGNAL(buttonClicked(int)), this, SLOT(UpdateState(int)));
	connect(ui->resetToParentButton, SIGNAL(clicked()), this, SLOT(ResetAggregatorSize()));
	connect(ui->aggregatorWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnSizeChanged()));
	connect(ui->aggregatorHeightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnSizeChanged()));

	QColor color;
	if (id % 2 == 0)
	{
		color = EVEN_COLOR;
	}
	else
	{
		color = ODD_COLOR;
	}

	QPalette pal(palette());
	pal.setColor(QPalette::Background, color);
	setAutoFillBackground(true);
	setPalette(pal);

	UpdateState(activeAction);
}

ImportDialogFileWidget::~ImportDialogFileWidget()
{
	delete ui;
}

void ImportDialogFileWidget::SetSizeWidgetShowable(bool showable)
{
	neverShowSizeWidget = !showable;
}

ImportDialog::eAction ImportDialogFileWidget::GetSelectedAction() const
{
	return activeAction;
}

void ImportDialogFileWidget::SetAction(ImportDialog::eAction action)
{
	if (activeAction == action)
	{
		return;
	}

	UpdateState(action);
}

void ImportDialogFileWidget::UpdateState(int action)
{
	ui->actionButtons->blockSignals(true);

	QPalette pal = ui->fileNameLabel->palette();

	activeAction = (ImportDialog::eAction)action;
	switch (activeAction)
	{
		case ImportDialog::ACTION_IGNORE:
			ui->ignoreRadioButton->setChecked(true);
			ui->actionLabel->setPixmap(QIcon(IconHelper::GetIgnoreIconPath()).pixmap(16, 16));
			ui->actionLabel->setToolTip(tr("Ignore file"));
			ui->aggregatorSizeWidget->hide();
			pal.setColor(QPalette::Foreground, IGNORED_FILE_COLOR);
			break;

		case ImportDialog::ACTION_SCREEN:
			ui->screenRadioButton->setChecked(true);
			ui->actionLabel->setPixmap(QIcon(IconHelper::GetScreenIconPath()).pixmap(16, 16));
			ui->actionLabel->setToolTip(tr("Load as Screen"));
			ui->aggregatorSizeWidget->hide();
			pal.setColor(QPalette::Foreground, ACTIVE_FILE_COLOR);
			break;

		case ImportDialog::ACTION_AGGREGATOR:
			ui->aggregatorRadioButton->setChecked(true);
			ui->actionLabel->setToolTip(tr("Load as Aggregator"));
			ui->actionLabel->setPixmap(QIcon(IconHelper::GetAggregatorIconPath()).pixmap(16, 16));

			if (!neverShowSizeWidget)
			{
				ui->aggregatorSizeWidget->show();
			}

			pal.setColor(QPalette::Foreground, ACTIVE_FILE_COLOR);
			break;

		default:
			break;
	}

	ui->upperIgnoreIcon->setVisible(showUpperIcons);
	ui->upperScreenIcon->setVisible(showUpperIcons);
	ui->upperAggregatorIcon->setVisible(showUpperIcons);

	if (showUpperIcons)
	{
		ui->actionLabel->setPixmap(QIcon().pixmap(16, 16));
	}

	ui->fileNameLabel->setPalette(pal);
	ui->actionButtons->blockSignals(false);

	emit ActionChanged(id);
}

QString ImportDialogFileWidget::GetFilename() const
{
	return filename;
}

QSize ImportDialogFileWidget::GetSize() const
{
	return QSize(ui->aggregatorWidthSpinBox->value(), ui->aggregatorHeightSpinBox->value());
}

void ImportDialogFileWidget::SetFilename(const QString& filename)
{
	this->filename = filename;
	ui->fileNameLabel->setText(filename);
}

void ImportDialogFileWidget::InitWithFilenameAndAction(const QString& filename, ImportDialog::eAction action)
{
	SetFilename(filename);
	UpdateState(action);
}

void ImportDialogFileWidget::ResetAggregatorSize()
{
	ui->aggregatorWidthSpinBox->setValue(-1);
	ui->aggregatorHeightSpinBox->setValue(-1);
}

void ImportDialogFileWidget::OnSizeChanged()
{
	emit SizeChanged();
}

void ImportDialogFileWidget::SetUpperIconsShowable(bool showable)
{
	showUpperIcons = showable;
	UpdateState(activeAction);
}
