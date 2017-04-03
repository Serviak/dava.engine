#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyModel.h"
#include "TArc/Controls/PropertyPanel/PropertyPanelMeta.h"
#include "TArc/Controls/QtBoxLayouts.h"

#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Utils/ScopedValueGuard.h"
#include "TArc/Utils/ReflectionHelpers.h"

#include <Engine/PlatformApi.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Logger/Logger.h>

#include <QApplication>
#include <QtEvents>
#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QToolButton>
#include <QPainter>

namespace DAVA
{
namespace TArc
{
BaseComponentValue::BaseComponentValue()
{
    thisValue = this;
}

BaseComponentValue::~BaseComponentValue()
{
    if (editorWidget != nullptr)
    {
        editorWidget->TearDown();
        editorWidget = nullptr;

        realWidget->deleteLater();
        realWidget = nullptr;
    }
}

void BaseComponentValue::Init(ReflectedPropertyModel* model_)
{
    model = model_;
}

void BaseComponentValue::Draw(QPainter* painter, const QStyleOptionViewItem& opt)
{
    UpdateEditorGeometry(opt.rect);
    realWidget->show();
}

void BaseComponentValue::UpdateGeometry(const QStyleOptionViewItem& opt)
{
    UpdateEditorGeometry(opt.rect);
}

bool BaseComponentValue::HasHeightForWidth() const
{
    DVASSERT(realWidget != nullptr);
    return realWidget->hasHeightForWidth();
}

int BaseComponentValue::GetHeightForWidth(int width) const
{
    DVASSERT(realWidget != nullptr);
    return realWidget->heightForWidth(width);
}

int BaseComponentValue::GetHeight() const
{
    DVASSERT(realWidget != nullptr);
    return realWidget->sizeHint().height();
}

QWidget* BaseComponentValue::AcquireEditorWidget(const QStyleOptionViewItem& option)
{
    UpdateEditorGeometry(option.rect);
    return realWidget;
}

QString BaseComponentValue::GetPropertyName() const
{
    const Reflection& r = nodes.front()->field.ref;
    const M::DisplayName* displayName = r.GetMeta<M::DisplayName>();
    if (displayName != nullptr)
    {
        return QString::fromStdString(displayName->displayName);
    }

    return nodes.front()->field.key.Cast<QString>();
}

FastName BaseComponentValue::GetName() const
{
    return nodes.front()->field.key.Cast<FastName>(FastName(""));
}

int32 BaseComponentValue::GetPropertiesNodeCount() const
{
    return static_cast<int32>(nodes.size());
}

std::shared_ptr<PropertyNode> BaseComponentValue::GetPropertyNode(int32 index) const
{
    DVASSERT(static_cast<size_t>(index) < nodes.size());
    return nodes[static_cast<size_t>(index)];
}

void BaseComponentValue::ForceUpdate()
{
    if (editorWidget != nullptr)
    {
        editorWidget->ForceUpdate();
    }
}

void BaseComponentValue::HideEditor()
{
    if (realWidget != nullptr)
    {
        realWidget->hide();
    }
}

bool BaseComponentValue::IsReadOnly() const
{
    Reflection r = nodes.front()->field.ref;
    return r.IsReadonly() || r.HasMeta<M::ReadOnly>();
}

bool BaseComponentValue::IsSpannedControl() const
{
    return false;
}

const BaseComponentValue::Style& BaseComponentValue::GetStyle() const
{
    return style;
}

void BaseComponentValue::SetStyle(const Style& style_)
{
    style = style_;
}

DAVA::Any BaseComponentValue::GetValue() const
{
    Any value = nodes.front()->cachedValue;
    for (const std::shared_ptr<const PropertyNode>& node : nodes)
    {
        if (value != node->cachedValue)
        {
            return GetMultipleValue();
        }
    }

    return value;
}

void BaseComponentValue::SetValue(const Any& value)
{
    if (IsValidValueToSet(value, GetValue()))
    {
        GetModifyInterface()->ModifyPropertyValue(nodes, value);
    }
}

std::shared_ptr<ModifyExtension> BaseComponentValue::GetModifyInterface()
{
    return model->GetExtensionChain<ModifyExtension>();
}

void BaseComponentValue::AddPropertyNode(const std::shared_ptr<PropertyNode>& node)
{
    nodes.push_back(node);
}

void BaseComponentValue::RemovePropertyNode(const std::shared_ptr<PropertyNode>& node)
{
    auto iter = std::find(nodes.begin(), nodes.end(), node);
    if (iter == nodes.end())
    {
        DVASSERT(false);
        return;
    }

    nodes.erase(iter);
}

void BaseComponentValue::RemovePropertyNodes()
{
    nodes.clear();
}

ContextAccessor* BaseComponentValue::GetAccessor() const
{
    return model->accessor;
}

UI* BaseComponentValue::GetUI() const
{
    return model->ui;
}

const WindowKey& BaseComponentValue::GetWindowKey() const
{
    return model->wndKey;
}

DAVA::TArc::DataWrappersProcessor* BaseComponentValue::GetDataProcessor() const
{
    return model->GetWrappersProcessor(nodes.front());
}

QWidget* BaseComponentValue::EnsureEditorCreated(QObject* eventFilter, QWidget* parent)
{
    if (editorWidget == nullptr)
    {
        EnsureEditorCreated(parent);
        realWidget->installEventFilter(eventFilter);
    }

    DVASSERT(realWidget->parent() == parent);
    return realWidget;
}

void BaseComponentValue::EnsureEditorCreated(QWidget* parent)
{
    editorWidget = CreateEditorWidget(parent, Reflection::Create(&thisValue), GetDataProcessor());
    editorWidget->ForceUpdate();
    realWidget = editorWidget->ToWidgetCast();

    const M::CommandProducerHolder* typeProducer = GetTypeMeta<M::CommandProducerHolder>(nodes.front()->cachedValue);
    const M::CommandProducerHolder* fieldProducer = nodes.front()->field.ref.GetMeta<M::CommandProducerHolder>();
    bool realProperty = nodes.front()->propertyType == PropertyNode::RealProperty;
    if (realProperty == true && (fieldProducer != nullptr || typeProducer != nullptr))
    {
        QWidget* boxWidget = new QWidget(parent);
        QtHBoxLayout* layout = new QtHBoxLayout(boxWidget);
        layout->setMargin(0);
        layout->setSpacing(1);

        CreateButtons(layout, typeProducer, true);
        CreateButtons(layout, fieldProducer, false);
        layout->addWidget(realWidget);

        realWidget = boxWidget;
    }
}

void BaseComponentValue::UpdateEditorGeometry(const QRect& geometry) const
{
    DVASSERT(realWidget != nullptr);
    if (realWidget->geometry() != geometry)
    {
        realWidget->setGeometry(geometry);
    }
}

void BaseComponentValue::CreateButtons(QLayout* layout, const M::CommandProducerHolder* holder, bool isTypeButtons)
{
    if (holder == nullptr)
    {
        return;
    }

    const Vector<std::shared_ptr<M::CommandProducer>>& commands = holder->GetCommandProducers();
    for (size_t i = 0; i < commands.size(); ++i)
    {
        bool createButton = false;
        const std::shared_ptr<M::CommandProducer>& cmd = commands[i];
        for (const std::shared_ptr<PropertyNode>& node : nodes)
        {
            if (cmd->IsApplyable(node))
            {
                createButton = true;
                break;
            }
        }

        if (createButton == true)
        {
            M::CommandProducer::Info info = cmd->GetInfo();
            QToolButton* button = new QToolButton(layout->widget());
            button->setIcon(info.icon);
            button->setToolTip(info.tooltip);
            button->setIconSize(toolButtonIconSize);
            button->setAutoRaise(true);
            if (cmd->OnlyForSingleSelection() && nodes.size() > 1)
            {
                button->setEnabled(false);
            }

            if (isTypeButtons == true)
            {
                connections.AddConnection(button, &QToolButton::clicked, [this, i]()
                                          {
                                              OnTypeButtonClicked(static_cast<int32>(i));
                                          });
            }
            else
            {
                connections.AddConnection(button, &QToolButton::clicked, [this, i]()
                                          {
                                              OnFieldButtonClicked(static_cast<int32>(i));
                                          });
            }

            layout->addWidget(button);
        }
    }
}

void BaseComponentValue::OnFieldButtonClicked(int32 index)
{
    const M::CommandProducerHolder* holder = nodes.front()->field.ref.GetMeta<M::CommandProducerHolder>();
    CallButtonAction(holder, index);
}

void BaseComponentValue::OnTypeButtonClicked(int32 index)
{
    const M::CommandProducerHolder* holder = GetTypeMeta<M::CommandProducerHolder>(nodes.front()->cachedValue);
    DVASSERT(holder);
    CallButtonAction(holder, index);
}

void BaseComponentValue::CallButtonAction(const M::CommandProducerHolder* holder, int32 index)
{
    DVASSERT(holder != nullptr);
    const Vector<std::shared_ptr<M::CommandProducer>>& producers = holder->GetCommandProducers();
    DVASSERT(index < static_cast<size_t>(producers.size()));

    std::shared_ptr<M::CommandProducer> producer = producers[index];
    M::CommandProducer::Info info = producer->GetInfo();
    producer->CreateCache(model->accessor);

    ModifyExtension::MultiCommandInterface cmdInterface = GetModifyInterface()->GetMultiCommandInterface(info.description, static_cast<uint32>(nodes.size()));
    for (std::shared_ptr<PropertyNode>& node : nodes)
    {
        if (producer->IsApplyable(node))
        {
            M::CommandProducer::Params params;
            params.accessor = model->accessor;
            params.invoker = model->invoker;
            params.ui = model->ui;
            cmdInterface.Exec(producer->CreateCommand(node, params));
        }
    }
    producer->ClearCache();
}

QSize BaseComponentValue::toolButtonIconSize = QSize(12, 12);

const char* BaseComponentValue::readOnlyFieldName = "isReadOnly";

DAVA_VIRTUAL_REFLECTION_IMPL(BaseComponentValue)
{
    ReflectionRegistrator<BaseComponentValue>::Begin()
    .Field(readOnlyFieldName, &BaseComponentValue::IsReadOnly, nullptr)
    .End();
}
}
}
