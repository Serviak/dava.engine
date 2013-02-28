#include "UISpinner.h"

namespace DAVA 
{

REGISTER_CLASS(UISpinner);

//use these names for children buttons to define UISpinner in .yaml
static const String BUTTON_NEXT_NAME = "buttonNext";
static const String BUTTON_PREVIOUS_NAME = "buttonPrevious";

void SpinnerAdapter::AddDelegate(SelectionDelegate* aDelegate)
{
    delegates.insert(aDelegate);
}

void SpinnerAdapter::RemoveDelegate(SelectionDelegate* aDelegate)
{
    delegates.erase(aDelegate);
}

void SpinnerAdapter::NotifyDelegates(bool isSelectedFirst, bool isSelectedLast, bool isSelectedChanged)
{
    Set<SelectionDelegate*>::const_iterator end = delegates.end();
    for (Set<SelectionDelegate*>::iterator it = delegates.begin(); it != end; ++it)
    {
        (*it)->OnSelectedChanged(isSelectedFirst, isSelectedLast, isSelectedChanged);
    }
}

bool SpinnerAdapter::Next()
{
    bool completedOk = SelectNext();
    if (completedOk)
        NotifyDelegates(false /*as we selected next it can't be first*/, IsSelectedLast(), true);
    return completedOk;
}

bool SpinnerAdapter::Previous()
{
    bool completedOk = SelectPrevious();
    if (completedOk)
        NotifyDelegates(IsSelectedFirst(), false /*as we selected previous it can't be last*/, true);
    return completedOk;
}


UISpinner::UISpinner(const Rect &rect, bool rectInAbsoluteCoordinates/* = FALSE*/) 
    : UIControl(rect, rectInAbsoluteCoordinates)
    , buttonNext(new UIButton())
    , buttonPrevious(new UIButton())
    , adapter(NULL)
{
    buttonNext->SetName(BUTTON_NEXT_NAME);
    buttonPrevious->SetName(BUTTON_PREVIOUS_NAME);
    AddControl(buttonNext);
    AddControl(buttonPrevious);
    InitButtons();
}

UISpinner::~UISpinner()
{
    ReleaseButtons();
    if (adapter)
        adapter->RemoveDelegate(this);
    SafeRelease(adapter);
}

void UISpinner::InitButtons()
{
    buttonNext->SetDisabled(!adapter || adapter->IsSelectedLast());
    buttonPrevious->SetDisabled(!adapter || adapter->IsSelectedFirst());
    buttonNext->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UISpinner::OnNextPressed));
    buttonPrevious->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UISpinner::OnPreviousPressed));
}

void UISpinner::ReleaseButtons()
{
    buttonNext->RemoveEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UISpinner::OnNextPressed));
    buttonPrevious->RemoveEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UISpinner::OnPreviousPressed));
    SafeRelease(buttonNext);
    SafeRelease(buttonPrevious);
}

void UISpinner::LoadFromYamlNode(YamlNode * node, UIYamlLoader * loader)
{
    //release default buttons - they have to be loaded from yaml
    RemoveControl(buttonNext);
    RemoveControl(buttonPrevious);
    ReleaseButtons();
    UIControl::LoadFromYamlNode(node, loader);
}

void UISpinner::LoadFromYamlNodeCompleted()
{
    UIControl * nextButtonControl = FindByName(BUTTON_NEXT_NAME);
    UIControl * previousButtonControl = FindByName(BUTTON_PREVIOUS_NAME);
    DVASSERT(nextButtonControl);
    DVASSERT(previousButtonControl);
    buttonNext = SafeRetain(DynamicTypeCheck<UIButton*>(nextButtonControl));
    buttonPrevious = SafeRetain(DynamicTypeCheck<UIButton*>(previousButtonControl));
    InitButtons();
}

void UISpinner::SetAdapter(SpinnerAdapter * anAdapter)
{
    if (adapter)
    {
        adapter->RemoveDelegate(this);
    }
    SafeRelease(adapter);

    adapter = SafeRetain(anAdapter);
    if (adapter)
    {
        buttonNext->SetDisabled(adapter->IsSelectedLast());
        buttonPrevious->SetDisabled(adapter->IsSelectedFirst());
        adapter->DisplaySelectedData(this);
        adapter->AddDelegate(this);
    }
    else
    {
        buttonNext->SetDisabled(true);
        buttonPrevious->SetDisabled(true);
    }
}

void UISpinner::OnNextPressed(DAVA::BaseObject * caller, void * param, void *callerData)
{
    //buttonNext is disabled if we have no adapter or selected adapter element is last, so we don't need checks here
    adapter->Next();
}    

void UISpinner::OnPreviousPressed(DAVA::BaseObject * caller, void * param, void *callerData)
{
    //buttonPrevious is disabled if we have no adapter or selected adapter element is first, so we don't need checks here
    adapter->Previous();
}

void UISpinner::OnSelectedChanged(bool isSelectedFirst, bool isSelectedLast, bool isSelectedChanged)
{
    buttonNext->SetDisabled(isSelectedLast);
    buttonPrevious->SetDisabled(isSelectedFirst);
    if (isSelectedChanged)
    {
        adapter->DisplaySelectedData(this);
    }
}

}