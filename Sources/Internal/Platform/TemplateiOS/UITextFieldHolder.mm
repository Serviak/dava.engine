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

#include "UITextFieldHolder.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "UI/UITextFieldiPhone.h"
#include "Core/Core.h"

#import <HelperAppDelegate.h>

@implementation UITextFieldHolder

@synthesize davaTextField;

- (id) init
{
	if (self = [super init])
	{
        DAVA::float32 divider = [HelperAppDelegate GetScale];
        
        self.bounds = CGRectMake(0.0f, 0.0f, DAVA::Core::Instance()->GetPhysicalScreenWidth()/divider, DAVA::Core::Instance()->GetPhysicalScreenHeight()/divider);
        
		self.center = CGPointMake(DAVA::Core::Instance()->GetPhysicalScreenWidth()/2/divider, DAVA::Core::Instance()->GetPhysicalScreenHeight()/2/divider);
		self.userInteractionEnabled = TRUE;
		textInputAllowed = YES;

        textField = [[UITextField alloc] initWithFrame: CGRectMake(0.f, 0.f, 0.f, 0.f)];
		textField.delegate = self;
		
		[self setupTraits];
        
        textField.userInteractionEnabled = NO;

		// Done!
		[self addSubview:textField];
	}
	return self;
}

- (void) setTextField:(DAVA::UITextField *) tf
{
    cppTextField = tf;
    if(tf)
    {
        const DAVA::Rect rect = tf->GetRect();
        textField.frame = CGRectMake((rect.x - DAVA::Core::Instance()->GetVirtualScreenXMin()) * DAVA::Core::GetVirtualToPhysicalFactor()
                                     , (rect.y - DAVA::Core::Instance()->GetVirtualScreenYMin()) * DAVA::Core::GetVirtualToPhysicalFactor()
                                     , rect.dx * DAVA::Core::GetVirtualToPhysicalFactor()
                                     , rect.dy * DAVA::Core::GetVirtualToPhysicalFactor());
    }
    else
    {
        textField.frame = CGRectMake(0.f, 0.f, 0.f, 0.f);
    }
}


-(void)textFieldDidBeginEditing:(UITextField *)textField
{
    if (cppTextField)
    {
        if (DAVA::UIControlSystem::Instance()->GetFocusedControl() != cppTextField)
        {
            DAVA::UIControlSystem::Instance()->SetFocusedControl(cppTextField, false);
        }
    }
}

-(id)hitTest:(CGPoint)point withEvent:(UIEvent *)event
{
    id hitView = [super hitTest:point withEvent:event];
    if (hitView == self) return nil;
    else return hitView;
}

- (void) dealloc
{
	[textField release];
	textField = 0;
	[[NSNotificationCenter defaultCenter] removeObserver:self];

	[super dealloc];
}


- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
	if (cppTextField)
	{
		if (cppTextField->GetDelegate() != 0)
			cppTextField->GetDelegate()->TextFieldShouldReturn(cppTextField);
	}
	return TRUE;
}

- (BOOL)textField:(UITextField *)_textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string
{
	if (cppTextField && (cppTextField->GetDelegate() != 0))
    {
        // Check length limits.
        BOOL needIgnoreDelegateResult = FALSE;
        int maxLength = cppTextField->GetMaxLength();
        if (maxLength >= 0)
        {
            NSUInteger curLength = [textField.text length];
            NSUInteger newLength = curLength - range.length + [string length];
            if (newLength > (NSUInteger)maxLength)
            {
                NSUInteger charsToInsert = 0;
                if (range.length == 0)
                {
                    // Inserting without replace.
                    charsToInsert = (NSUInteger)maxLength - curLength;
                }
                else
                {
                    // Inserting with replace.
                    charsToInsert = range.length;
                }

                string = [string substringToIndex:charsToInsert];
                [textField setText: [textField.text stringByReplacingCharactersInRange:range withString:string]];

                needIgnoreDelegateResult = TRUE;
            }
        }

        // Length check OK, continue with the delegate.
        DAVA::WideString repString;
        const char * cstr = [string cStringUsingEncoding:NSUTF8StringEncoding];
        DAVA::UTF8Utils::EncodeToWideString((DAVA::uint8*)cstr, (DAVA::int32)strlen(cstr), repString);

        BOOL delegateResult = cppTextField->GetDelegate()->TextFieldKeyPressed(cppTextField, (DAVA::int32)range.location, (DAVA::int32)range.length, repString);
        return needIgnoreDelegateResult ? FALSE : delegateResult;
	}

	return TRUE;
}

- (BOOL)textFieldShouldBeginEditing:(UITextField *)textField
{
	return textInputAllowed;
}

- (void)setIsPassword:(bool)isPassword
{
	[textField setSecureTextEntry:isPassword ? YES: NO];
}

- (void)setTextInputAllowed:(bool)value
{
	textInputAllowed = (value == true);
}

- (void) setupTraits
{
	if (!cppTextField || !textField)
	{
		return;
	}

	textField.autocapitalizationType = [self convertAutoCapitalizationType: (DAVA::UITextField::eAutoCapitalizationType)cppTextField->GetAutoCapitalizationType()];
	textField.autocorrectionType = [self convertAutoCorrectionType: (DAVA::UITextField::eAutoCorrectionType)cppTextField->GetAutoCorrectionType()];
	
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_5_0
	textField.spellCheckingType = [self convertSpellCheckingType: cppTextField->GetSpellCheckingType()];
#endif
	textField.enablesReturnKeyAutomatically = [self convertEnablesReturnKeyAutomatically: cppTextField->IsEnableReturnKeyAutomatically()];
	textField.keyboardAppearance = [self convertKeyboardAppearanceType: (DAVA::UITextField::eKeyboardAppearanceType)cppTextField->GetKeyboardAppearanceType()];
	textField.keyboardType = [self convertKeyboardType: (DAVA::UITextField::eKeyboardType)cppTextField->GetKeyboardType()];
	textField.returnKeyType = [self convertReturnKeyType: (DAVA::UITextField::eReturnKeyType)cppTextField->GetReturnKeyType()];
}

- (UITextAutocapitalizationType) convertAutoCapitalizationType:(DAVA::UITextField::eAutoCapitalizationType) davaType
{
	switch (davaType)
	{
		case DAVA::UITextField::AUTO_CAPITALIZATION_TYPE_NONE:
		{
			return UITextAutocapitalizationTypeNone;
		}

		case DAVA::UITextField::AUTO_CAPITALIZATION_TYPE_WORDS:
		{
			return UITextAutocapitalizationTypeWords;
		}

		case DAVA::UITextField::AUTO_CAPITALIZATION_TYPE_ALL_CHARS:
		{
			return UITextAutocapitalizationTypeAllCharacters;
		}

		case DAVA::UITextField::AUTO_CAPITALIZATION_TYPE_SENTENCES:
		default:
		{
			// This is default one for iOS.
			return UITextAutocapitalizationTypeSentences;
		}
	}
}

- (UITextAutocorrectionType) convertAutoCorrectionType:(DAVA::UITextField::eAutoCorrectionType) davaType
{
	switch (davaType)
	{
		case DAVA::UITextField::AUTO_CORRECTION_TYPE_NO:
		{
			return UITextAutocorrectionTypeNo;
		}
			
		case DAVA::UITextField::AUTO_CORRECTION_TYPE_YES:
		{
			
			return UITextAutocorrectionTypeYes;
		}

		case DAVA::UITextField::AUTO_CORRECTION_TYPE_DEFAULT:
		default:
		{
			return UITextAutocorrectionTypeDefault;
		}
	}
}

- (UITextSpellCheckingType) convertSpellCheckingType:(DAVA::UITextField::eSpellCheckingType) davaType
{
	switch (davaType)
	{
		case DAVA::UITextField::SPELL_CHECKING_TYPE_NO:
		{
			return UITextSpellCheckingTypeNo;
		}

		case DAVA::UITextField::SPELL_CHECKING_TYPE_YES:
		{
			return UITextSpellCheckingTypeYes;
		}

		case DAVA::UITextField::SPELL_CHECKING_TYPE_DEFAULT:
		default:
		{
			return UITextSpellCheckingTypeDefault;
		}
	}
}

- (BOOL) convertEnablesReturnKeyAutomatically:(bool) davaType
{
	return (davaType ? YES : NO);
}

- (UIKeyboardAppearance) convertKeyboardAppearanceType:(DAVA::UITextField::eKeyboardAppearanceType) davaType
{
	switch (davaType)
	{
		case DAVA::UITextField::KEYBOARD_APPEARANCE_ALERT:
		{
			return UIKeyboardAppearanceAlert;
		}
			
		case DAVA::UITextField::KEYBOARD_APPEARANCE_DEFAULT:
		default:
		{
			return UIKeyboardAppearanceDefault;
		}
	}
}

- (UIKeyboardType) convertKeyboardType:(DAVA::UITextField::eKeyboardType) davaType
{
	switch (davaType)
	{
		case DAVA::UITextField::KEYBOARD_TYPE_ASCII_CAPABLE:
		{
			return UIKeyboardTypeASCIICapable;
		}

		case DAVA::UITextField::KEYBOARD_TYPE_NUMBERS_AND_PUNCTUATION:
		{
			return UIKeyboardTypeNumbersAndPunctuation;
		}

		case DAVA::UITextField::KEYBOARD_TYPE_URL:
		{
			return UIKeyboardTypeURL;
		}

		case DAVA::UITextField:: KEYBOARD_TYPE_NUMBER_PAD:
		{
			return UIKeyboardTypeNumberPad;
		}

		case DAVA::UITextField::KEYBOARD_TYPE_PHONE_PAD:
		{
			return UIKeyboardTypePhonePad;
		}

		case DAVA::UITextField::KEYBOARD_TYPE_NAME_PHONE_PAD:
		{
			return UIKeyboardTypeNamePhonePad;
		}

		case DAVA::UITextField::KEYBOARD_TYPE_EMAIL_ADDRESS:
		{
			return UIKeyboardTypeEmailAddress;
		}

		case DAVA::UITextField::KEYBOARD_TYPE_DECIMAL_PAD:
		{
			return UIKeyboardTypeDecimalPad;
		}

		case DAVA::UITextField::KEYBOARD_TYPE_TWITTER:
		{
			return UIKeyboardTypeTwitter;
		}

		case DAVA::UITextField::KEYBOARD_TYPE_DEFAULT:
		default:
		{
			return UIKeyboardTypeDefault;
		}
	}
}

- (UIReturnKeyType) convertReturnKeyType:(DAVA::UITextField::eReturnKeyType) davaType
{
	switch (davaType)
	{
		case DAVA::UITextField::RETURN_KEY_GO:
		{
			return UIReturnKeyGo;
		}

		case DAVA::UITextField::RETURN_KEY_GOOGLE:
		{
			return UIReturnKeyGoogle;
		}

		case DAVA::UITextField::RETURN_KEY_JOIN:
		{
			return UIReturnKeyJoin;
		}

		case DAVA::UITextField::RETURN_KEY_NEXT:
		{
			return UIReturnKeyNext;
		}

		case DAVA::UITextField::RETURN_KEY_ROUTE:
		{
			return UIReturnKeyRoute;
		}

		case DAVA::UITextField::RETURN_KEY_SEARCH:
		{
			return UIReturnKeySearch;
		}

		case DAVA::UITextField::RETURN_KEY_SEND:
		{
			return UIReturnKeySend;
		}

		case DAVA::UITextField::RETURN_KEY_YAHOO:
		{
			return UIReturnKeyYahoo;
		}

		case DAVA::UITextField::RETURN_KEY_DONE:
		{
			return UIReturnKeyDone;
		}

		case DAVA::UITextField::RETURN_KEY_EMERGENCY_CALL:
		{
			return UIReturnKeyEmergencyCall;
		}

		case DAVA::UITextField::RETURN_KEY_DEFAULT:
		default:
		{
			return UIReturnKeyDefault;
		}
	}
}

- (void)keyboardFrameDidChange:(NSNotification *)notification
{
    NSDictionary* userInfo = notification.userInfo;

    // Remember the last keyboard frame here, since it might be incorrect in keyboardDidShow.
    lastKeyboardFrame = [[notification.userInfo objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
}

- (void)keyboardWillHide:(NSNotification *)notification
{
	if (cppTextField && cppTextField->GetDelegate())
	{
		cppTextField->GetDelegate()->OnKeyboardHidden();
	}
}

- (void)keyboardDidShow:(NSNotification *)notification
{
	if (!cppTextField || !cppTextField->GetDelegate())
	{
		return;
	}

	// convert own frame to window coordinates, frame is in superview's coordinates
	CGRect ownFrame = [textField.window convertRect:self.frame fromView:textField.superview];

	// calculate the area of own frame that is covered by keyboard
	CGRect keyboardFrame = CGRectIntersection(ownFrame, lastKeyboardFrame);

	// now this might be rotated, so convert it back
	keyboardFrame = [textField.window convertRect:keyboardFrame toView:textField.superview];

	// Recalculate to virtual coordinates.
	DAVA::Vector2 keyboardOrigin(keyboardFrame.origin.x, keyboardFrame.origin.y);
	keyboardOrigin *= DAVA::UIControlSystem::Instance()->GetScaleFactor();
	keyboardOrigin += DAVA::UIControlSystem::Instance()->GetInputOffset();
	
	DAVA::Vector2 keyboardSize(keyboardFrame.size.width, keyboardFrame.size.height);
	keyboardSize *= DAVA::UIControlSystem::Instance()->GetScaleFactor();
	keyboardSize += DAVA::UIControlSystem::Instance()->GetInputOffset();

	cppTextField->GetDelegate()->OnKeyboardShown(DAVA::Rect(keyboardOrigin, keyboardSize));
}

@end

#endif //#if defined(__DAVAENGINE_IPHONE__)
