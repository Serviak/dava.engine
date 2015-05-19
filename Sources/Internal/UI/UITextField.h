/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#ifndef __DAVAENGINE_UI_TEXT_FIELD_H__
#define __DAVAENGINE_UI_TEXT_FIELD_H__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"
#include "UI/UIStaticText.h"
#include "UI/UIControlSystem.h"

#ifdef __DAVAENGINE_IPHONE__
#include "UI/UITextFieldiPhone.h"
#endif

namespace DAVA 
{

class UITextField;
#ifdef __DAVAENGINE_ANDROID__
class UITextFieldAndroid;
#endif
/**
    \brief  The UITextFieldDelegate interface defines the messages sent to a text field delegate as part of the sequence of editing its text. 
            All the methods of the interface is optional.
 */
class UITextFieldDelegate
{
public:
	/*virtual void TextFieldShouldBeginEditing(UITextField * textField);
	virtual void TextFieldDidBeginEditing(UITextField * textField);
	virtual void TextFieldShouldEndEditing(UITextField * textField);
	virtual void TextFieldShouldDidEditing(UITextField * textField);*/
	
    /**
        \brief Asks the delegate if the text field should process the pressing of the return button.
        In this function you can check what you want to do with UITextField when return button pressed. 
     */
	virtual void TextFieldShouldReturn(UITextField * textField);

    /**
        \brief Asks the delegate if the text field should process the pressing of the ESC button.
        In this function you can check what you want to do with UITextField when ESC button pressed.
        Don't work on iOS for now.
     */
	virtual void TextFieldShouldCancel(UITextField * textField);
	virtual void TextFieldLostFocus(UITextField * textField);

	/**
        \brief Asks the delegate if the specified text should be changed.
        \param[in] textField The text field containing the text.
        \param[in] replacementLocation starting position of range of characters to be replaced
        \param[in] replacementLength ending position of range of characters to be replaced
        \param[in] replacementString the replacement string.
        \returns true if the specified text range should be replaced; otherwise, false to keep the old text. Default implementation returns true.
	 */
	virtual bool TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, int32 replacementLength, WideString & replacementString);

    virtual void TextFieldOnTextChanged(UITextField * textField, const WideString& newText, const WideString& oldText);
    
    virtual bool IsTextFieldShouldSetFocusedOnAppear(UITextField * textField);
    virtual bool IsTextFieldCanLostFocus(UITextField * textField);
	
	/*
	 \brief Called when device keyboard is displayed/hidden.
	 */
	virtual void OnKeyboardShown(const Rect& keyboardRect);
	virtual void OnKeyboardHidden();
};
    
/**
    \brief  A UITextField object is a control that displays editable text and sends an action message to a target object when the user presses the return button. 
            You typically use this class to gather small amounts of text from the user and perform some immediate action, such as a search operation, based on that text.
            A text field object supports the use of a delegate object to handle editing-related notifications. 
 */
class UITextField : public UIControl 
{
public:
	// Auto-capitalization type.
	enum eAutoCapitalizationType
	{
		AUTO_CAPITALIZATION_TYPE_NONE = 0,
		AUTO_CAPITALIZATION_TYPE_WORDS,
		AUTO_CAPITALIZATION_TYPE_SENTENCES,
		AUTO_CAPITALIZATION_TYPE_ALL_CHARS,
	};
	
	// Auto-correction type.
	enum eAutoCorrectionType
	{
		AUTO_CORRECTION_TYPE_DEFAULT = 0,
		AUTO_CORRECTION_TYPE_NO,
		AUTO_CORRECTION_TYPE_YES
	};
	
	// Spell checking type.
	enum eSpellCheckingType
	{
		SPELL_CHECKING_TYPE_DEFAULT = 0,
		SPELL_CHECKING_TYPE_NO,
		SPELL_CHECKING_TYPE_YES
	};
	
	// Keyboard appearance.
	enum eKeyboardAppearanceType
	{
		KEYBOARD_APPEARANCE_DEFAULT = 0,
		KEYBOARD_APPEARANCE_ALERT
	};

	// Keyboard type.
	enum eKeyboardType
	{
		KEYBOARD_TYPE_DEFAULT = 0,
		KEYBOARD_TYPE_ASCII_CAPABLE,
		KEYBOARD_TYPE_NUMBERS_AND_PUNCTUATION,
		KEYBOARD_TYPE_URL,
		KEYBOARD_TYPE_NUMBER_PAD,
		KEYBOARD_TYPE_PHONE_PAD,
		KEYBOARD_TYPE_NAME_PHONE_PAD,
		KEYBOARD_TYPE_EMAIL_ADDRESS,
		KEYBOARD_TYPE_DECIMAL_PAD,
		KEYBOARD_TYPE_TWITTER,
	};
	
	// Return key type.
	enum eReturnKeyType
	{
		RETURN_KEY_DEFAULT = 0,
		RETURN_KEY_GO,
		RETURN_KEY_GOOGLE,
		RETURN_KEY_JOIN,
		RETURN_KEY_NEXT,
		RETURN_KEY_ROUTE,
		RETURN_KEY_SEARCH,
		RETURN_KEY_SEND,
		RETURN_KEY_YAHOO,
		RETURN_KEY_DONE,
		RETURN_KEY_EMERGENCY_CALL
	};

protected:
	~UITextField() override;
public:
	UITextField(const Rect &rect = Rect(), bool rectInAbsoluteCoordinates = false);
	
	void WillAppear() override;
	void DidAppear() override;
	void WillDisappear() override;
	
    void OnFocused() override;
    void OnFocusLost(UIControl* newFocus) override;

	void SetDelegate(UITextFieldDelegate* delegate);
	UITextFieldDelegate* GetDelegate();

	void Update(float32 timeElapsed) override;
	
	void OpenKeyboard();
	void CloseKeyboard();
	
	void SetSpriteAlign(int32 align) override;
    
	const WideString & GetText();
	virtual void SetText(const WideString& text);
    
    WideString GetAppliedChanges(int32 replacementLocation,
            int32 replacementLength, const WideString& replacementString);

    void Input(UIEvent* currentInput) override;

    void LoadFromYamlNode(const YamlNode* node, UIYamlLoader* loader) override;
    YamlNode * SaveToYamlNode(UIYamlLoader * loader) override;
	
	/**
	 \brief Sets contol input processing ability.
	 */
	void SetInputEnabled(bool isEnabled, bool hierarchic = true) override;

protected:
    void WillBecomeVisible() override;
    void WillBecomeInvisible() override;

public:
	/**
	 \brief Returns the font of control
	 \returns Font font of the control
	 */
    Font *GetFont() const;
	/**
	 \brief Returns the text color of control.
	 \returns Color color of control's text
	 */
	const Color &GetTextColor() const;
	/**
	 \brief Returns text shadow offset relative to base text.
	 \returns Vector2 with shadow offset for X and Y axis
	 */
	Vector2 GetShadowOffset() const;
	/**
	 \brief Returns color of text shadow.
	 \returns Color of text shadow.
	 */
	const Color &GetShadowColor() const;

	int32 GetTextAlign() const;

	/**
	 \brief Returns using RTL align flag
	 \returns Using RTL align flag
	 */
	bool GetTextUseRtlAlign() const;

    void SetFocused()
    {
        UIControlSystem::Instance()->SetFocusedControl(this, true);
    }
    
    void ReleaseFocus();
    
    bool IsLostFocusAllowed(UIControl *newFocus) override;
	
  	/**
	 \brief Sets the font of the control text.
	 \param[in] font font used for text draw of the states.
	 */  
    void SetFont(Font * font);
	/**
	 \brief Sets the color of the text.
	 \param[in] fontColor font used for text draw of the states.
	 */
    void SetTextColor(const Color& fontColor);
	/**
	 \brief Sets the size of the font.
	 \param[in] size font size to be set.
	 */
    void SetFontSize(float32 _size);
	/**
	 \brief Sets shadow offset of text control.
	 \param[in] offset offset of text shadow relative to base text.
	 */
	void SetShadowOffset(const DAVA::Vector2 &offset);
	/**
	 \brief Sets shadow color of text control.
	 \param[in] color color of text shadow.
	 */
	void SetShadowColor(const Color& color);

	void SetTextAlign(int32 align);

	/**
	 \brief Sets using mirror align for RTL texts
	 \param[in] useRrlAlign flag of support RTL align
	 */
	void SetTextUseRtlAlign(bool useRtlAlign);

    void SetSize(const DAVA::Vector2 &newSize) override;
    void SetPosition(const Vector2 &position) override;

    /**
	 \brief Set control text style hide.
	 \param[in] isPassword draw text with hide char.
	 */
    void SetIsPassword(bool isPassword);
    /**
	 \brief Return is text style is hide.
	 */
    bool IsPassword() const;

	/**
 	 \brief Auto-capitalization type.
	 */
	int32 GetAutoCapitalizationType() const;
	void SetAutoCapitalizationType(int32 value);

	/**
 	 \brief Auto-correction type.
	 */
	int32 GetAutoCorrectionType() const;
	void SetAutoCorrectionType(int32 value);

	/**
 	 \brief Spell checking type.
	 */
	int32 GetSpellCheckingType() const;
	void SetSpellCheckingType(int32 value);

	/**
 	 \brief Keyboard appearance type.
	 */
	int32 GetKeyboardAppearanceType() const;
	void SetKeyboardAppearanceType(int32 value);

	/**
 	 \brief Keyboard type.
	 */
	int32 GetKeyboardType() const;
	void SetKeyboardType(int32 value);
	
	/**
 	 \brief Return key type.
	 */
	int32 GetReturnKeyType() const;
	void SetReturnKeyType(int32 value);

	/**
 	 \brief Enable return key automatically.
	 */
	bool IsEnableReturnKeyAutomatically() const;
	void SetEnableReturnKeyAutomatically(bool value);
	
	/**
	 \brief Returns list of control children without internal controls.
	 \returns list of control children without internal controls.
	 */
	List<UIControl* >& GetRealChildren() override;
	
	UITextField *Clone() override;
	void CopyDataFrom(UIControl *srcControl) override;

    // Cursor control.
    uint32 GetCursorPos();
    void SetCursorPos(uint32 pos);

    /**
      \brief Set maximum text length in text edit
      maxLength - >=0 - max count, -1 - unlimited count
     */
    void SetMaxLength(int32 maxLength);
    int32 GetMaxLength() const;

    String GetFontPresetName() const;

    void SetFontByPresetName(const String &presetName);

protected:
	WideString text;
	UITextFieldDelegate * delegate;
	float32	cursorBlinkingTime;

    // Keyboard customization params.
	eAutoCapitalizationType autoCapitalizationType;
	eAutoCorrectionType autoCorrectionType;
	eSpellCheckingType spellCheckingType;
	eKeyboardAppearanceType keyboardAppearanceType;
	eKeyboardType keyboardType;
	eReturnKeyType returnKeyType;

	// All Boolean variables are grouped together because of DF-2149.
	bool needRedraw;
	bool isPassword;
	bool enableReturnKeyAutomatically;
	bool showCursor;
	bool isRenderToTexture;

    void RenderText();
private:
    WideString GetVisibleText() const;

    void SetRenderToTexture(bool value);
    bool IsRenderToTexture() const;

    
    /**
         \brief Setups initial state to reset settings for cached native control.
     */
    void SetupDefaults();

#ifdef __DAVAENGINE_IPHONE__
	UITextFieldiPhone * textFieldiPhone;
#elif defined(__DAVAENGINE_ANDROID__)
	UITextFieldAndroid* textFieldAndroid;
#else
    UIStaticText * staticText;
    Font * textFont;
#endif
    float32 cursorTime;
    int32 maxLength;
public:
    INTROSPECTION_EXTEND(UITextField, UIControl,
        PROPERTY("text", "Text", GetText, SetText, I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("font", "Font", GetFontPresetName, SetFontByPresetName, I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("textcolor", "Text color", GetTextColor, SetTextColor, I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("shadowoffset", "Shadow Offset", GetShadowOffset, SetShadowOffset, I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("shadowcolor", "Shadow Color", GetShadowColor, SetShadowColor, I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("textalign", InspDesc("Text Align", GlobalEnumMap<eAlign>::Instance(), InspDesc::T_FLAGS), GetTextAlign, SetTextAlign, I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("textUseRtlAlign", "Use Rtl Align", GetTextUseRtlAlign, SetTextUseRtlAlign, I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("maxLength", "Max text lenght", GetMaxLength, SetMaxLength, I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("isPassword", "Is password", IsPassword, SetIsPassword, I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("autoCapitalizationType", InspDesc("Auto capitalization type", GlobalEnumMap<eAutoCapitalizationType>::Instance()), GetAutoCapitalizationType, SetAutoCapitalizationType , I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("autoCorrectionType"    , InspDesc("Auto correction type"    , GlobalEnumMap<eAutoCorrectionType>::Instance())    , GetAutoCorrectionType    , SetAutoCorrectionType     , I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("spellCheckingType"     , InspDesc("Spell checking type"     , GlobalEnumMap<eSpellCheckingType>::Instance())     , GetSpellCheckingType     , SetSpellCheckingType      , I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("keyboardAppearanceType", InspDesc("Keyboard appearance type", GlobalEnumMap<eKeyboardAppearanceType>::Instance()), GetKeyboardAppearanceType, SetKeyboardAppearanceType , I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("keyboardType"          , InspDesc("Keyboard type"           , GlobalEnumMap<eKeyboardType>::Instance())          , GetKeyboardType          , SetKeyboardType           , I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("returnKeyType"         , InspDesc("Return key type"         , GlobalEnumMap<eReturnKeyType>::Instance())         , GetReturnKeyType         , SetReturnKeyType          , I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("enableReturnKeyAutomatically", "Automatically enable return key", IsEnableReturnKeyAutomatically, SetEnableReturnKeyAutomatically, I_SAVE | I_VIEW | I_EDIT)
    );
};

};

#endif // __DAVAENGINE_UI_TEXT_FIELD_H__
