//
//  InputTest.h
//  TemplateProjectMacOS
//
//  Created by adebt on 1/29/13.
//
//

#ifndef __TemplateProjectMacOS__InputTest__
#define __TemplateProjectMacOS__InputTest__

#include "DAVAEngine.h"
#include "UI/UIWebView.h"

using namespace DAVA;

#include "TestTemplate.h"

class InputTest: public TestTemplate<InputTest>, public UITextFieldDelegate
{
public:
	InputTest();

	virtual void LoadResources();
	virtual void UnloadResources();
	virtual bool RunTest(int32 testNum);

	virtual void DidAppear();
	virtual void Update(float32 timeElapsed);

	virtual bool TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, int32 replacementLength, const WideString & replacementString);

	void TestFunction(PerfFuncData * data);
	void OnPageLoaded(DAVA::BaseObject * caller, void * param, void *callerData);
	
private:
	void ButtonPressed(BaseObject *obj, void *data, void *callerData);
	
private:
	UITextField* textField;
	UITextField* passwordTextField;

	UIStaticText* staticText;
	UIButton* testButton;
	
	UIWebView* webView1;
	UIWebView* webView2;
	UIWebView* webView3;
	
	void* delegate;

	bool testFinished;
	float onScreenTime;
};

#endif /* defined(__TemplateProjectMacOS__InputTest__) */
