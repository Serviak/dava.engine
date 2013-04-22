//
//  ParticleEffectPropertiesWidget.cpp
//  ResourceEditorQt
//
//  Created by Yuri Coder on 4/11/13.
//
//

#include "ParticleEffectPropertiesWidget.h"
#include "Commands/ParticleEditorCommands.h"
#include "Commands/CommandsManager.h"

#include <QLineEdit>
#include <QEvent>

ParticleEffectPropertiesWidget::ParticleEffectPropertiesWidget(QWidget* parent) :
QWidget(parent)
{
	mainLayout = new QVBoxLayout();
	mainLayout->setAlignment(Qt::AlignTop);
	this->setLayout(mainLayout);
	
	effectPlaybackSpeedLabel = new QLabel("effect playback speed");
	mainLayout->addWidget(effectPlaybackSpeedLabel);
	
	effectPlaybackSpeed = new QSlider(Qt::Horizontal, this);
	effectPlaybackSpeed->setTracking(true);
	effectPlaybackSpeed->setRange(0, 4); // 25%, 50%, 100%, 200%, 400% - 5 values total.
	effectPlaybackSpeed->setTickPosition(QSlider::TicksBelow);
	effectPlaybackSpeed->setTickInterval(1);
	effectPlaybackSpeed->setSingleStep(1);
	mainLayout->addWidget(effectPlaybackSpeed);

	connect(effectPlaybackSpeed, SIGNAL(valueChanged(int)), this, SLOT(OnValueChanged()));

	particleEffect = NULL;
	blockSignals = false;
}

ParticleEffectPropertiesWidget::~ParticleEffectPropertiesWidget()
{
}

void ParticleEffectPropertiesWidget::InitWidget(QWidget *widget, bool connectWidget)
{
	mainLayout->addWidget(widget);
	if(connectWidget)
		connect(widget, SIGNAL(ValueChanged()), this, SLOT(OnValueChanged()));
}

void ParticleEffectPropertiesWidget::OnValueChanged()
{
	if(blockSignals)
		return;
	
	DVASSERT(particleEffect != 0);
	float playbackSpeed = ConvertFromSliderValueToPlaybackSpeed(effectPlaybackSpeed->value());
	
	CommandUpdateEffect* commandUpdateEffect = new CommandUpdateEffect(particleEffect);
	commandUpdateEffect->Init(playbackSpeed);
	CommandsManager::Instance()->ExecuteAndRelease(commandUpdateEffect);

	Init(particleEffect);
}

void ParticleEffectPropertiesWidget::Init(DAVA::ParticleEffectComponent *effect)
{
	DVASSERT(effect != 0);
	this->particleEffect = effect;
	this->emitter = NULL;

	blockSignals = true;

	// Normalize Playback Speed to the UISlider range.
	float32 playbackSpeed = particleEffect->GetPlaybackSpeed();
	effectPlaybackSpeed->setValue(ConvertFromPlaybackSpeedToSliderValue(playbackSpeed));
	UpdatePlaybackSpeedLabel();
	
	blockSignals = false;
}

void ParticleEffectPropertiesWidget::UpdatePlaybackSpeedLabel()
{
	if (!particleEffect)
	{
		return;
	}
	
	float32 playbackSpeedValue = particleEffect->GetPlaybackSpeed();
	effectPlaybackSpeedLabel->setText(QString("playback speed: %1x").arg(playbackSpeedValue));
}

void ParticleEffectPropertiesWidget::StoreVisualState(KeyedArchive* /* visualStateProps */)
{
	// Nothing to store for now.
}

void ParticleEffectPropertiesWidget::RestoreVisualState(KeyedArchive* /* visualStateProps */)
{
	// Nothing to restore for now.
}
