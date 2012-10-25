#include "TextureConverterDialog.h"
#include "ControlsFactory.h"

#include "TextureConverterCell.h"

#include "ErrorNotifier.h"
#include "PVRConverter.h"

#include "UIZoomControl.h"
#include "SceneValidator.h"

TextureConverterDialog::TextureConverterDialog(const Rect &rect, bool rectInAbsoluteCoordinates)
    :   UIControl(rect, rectInAbsoluteCoordinates)
{
    selectedItem = -1;
    selectedTextureName = "";
    
    workingScene = NULL;
    
    ControlsFactory::CustomizePanelControl(this);
    
    textureList = new UIList(Rect(0, 0, ControlsFactory::TEXTURE_PREVIEW_WIDTH, rect.dy), 
                             UIList::ORIENTATION_VERTICAL);
    ControlsFactory::SetScrollbar(textureList);
    textureList->SetDelegate(this);
    AddControl(textureList);
    
    float32 closeButtonSide = ControlsFactory::BUTTON_HEIGHT;
    closeButtonTop = ControlsFactory::CreateCloseWindowButton(Rect(rect.dx - closeButtonSide, 0, closeButtonSide, closeButtonSide));
    closeButtonTop->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &TextureConverterDialog::OnCancel));
    AddControl(closeButtonTop);
    
    
    UIStaticText *notification = new UIStaticText(Rect(ControlsFactory::TEXTURE_PREVIEW_WIDTH, 0, 
                                                       rect.dx - ControlsFactory::TEXTURE_PREVIEW_WIDTH - closeButtonSide,
                                                       closeButtonSide));
    notification->SetFont(ControlsFactory::GetFontError());
    notification->SetAlign(ALIGN_VCENTER | ALIGN_HCENTER);
    notification->SetText(LocalizedString(L"textureconverter.notification"));
    AddControl(notification);
    SafeRelease(notification);
    
    
    

    float32 x = rect.dx - ControlsFactory::BUTTON_WIDTH;
    convertButton = ControlsFactory::CreateButton(Vector2(x, 
                                                          rect.dy - (float32)ControlsFactory::BUTTON_HEIGHT), 
                                                  LocalizedString(L"textureconverter.convert"));
    convertButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &TextureConverterDialog::OnConvert));
    
    AddControl(convertButton);
    
    AddLine(Rect((float32)ControlsFactory::TEXTURE_PREVIEW_WIDTH, 0, 1.f, rect.dy));
    
    
    dstPreview = new UIControl(Rect(0, 0, 100, 100));
    dstPreview->SetInputEnabled(false);
    dstPreview->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_PROPORTIONAL);
    srcPreview = new UIControl(Rect(0, 0, 100, 100));
    srcPreview->SetInputEnabled(false);
    srcPreview->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_PROPORTIONAL);

    float32 width = rect.dx - ControlsFactory::TEXTURE_PREVIEW_WIDTH;
    float32 height = (rect.dy - ControlsFactory::BUTTON_HEIGHT*2.f);
    
    if(width < height)
    {
        width = Min(width, height/2.f);
        --width;

        Rect srcRect((float32)ControlsFactory::TEXTURE_PREVIEW_WIDTH, (float32)ControlsFactory::BUTTON_HEIGHT, width, width);
        Rect dstRect((float32)ControlsFactory::TEXTURE_PREVIEW_WIDTH, (float32)ControlsFactory::BUTTON_HEIGHT + width + 1.f, width, width);
        
        srcZoomPreview = new UIZoomControl(srcRect);
        dstZoomPreview = new UIZoomControl(dstRect);
        
        AddLine(Rect((float32)ControlsFactory::TEXTURE_PREVIEW_WIDTH+width, (float32)ControlsFactory::BUTTON_HEIGHT, 1.f, width * 2.f));
        AddLine(Rect((float32)ControlsFactory::TEXTURE_PREVIEW_WIDTH, (float32)ControlsFactory::BUTTON_HEIGHT + width, width, 1.f));
        AddLine(Rect((float32)ControlsFactory::TEXTURE_PREVIEW_WIDTH, (float32)ControlsFactory::BUTTON_HEIGHT + width*2.f, width, 1.f));
    }
    else 
    {
        height = Min(height, width/2.f);
        --height;
        Rect srcRect((float32)ControlsFactory::TEXTURE_PREVIEW_WIDTH, (float32)ControlsFactory::BUTTON_HEIGHT, height, height);
        Rect dstRect((float32)ControlsFactory::TEXTURE_PREVIEW_WIDTH + height + 1.f,(float32) ControlsFactory::BUTTON_HEIGHT, height, height);
        
        srcZoomPreview = new UIZoomControl(srcRect);
        dstZoomPreview = new UIZoomControl(dstRect);
        
        AddLine(Rect((float32)ControlsFactory::TEXTURE_PREVIEW_WIDTH, ControlsFactory::BUTTON_HEIGHT + height, height*2, 1));
        
        AddLine(Rect((float32)ControlsFactory::TEXTURE_PREVIEW_WIDTH + height, (float32)ControlsFactory::BUTTON_HEIGHT, 1.f, height));
        AddLine(Rect((float32)ControlsFactory::TEXTURE_PREVIEW_WIDTH + height*2.f, (float32)ControlsFactory::BUTTON_HEIGHT, 1.f, height));
    }
    
    srcZoomPreview->AddControl(srcPreview);
    dstZoomPreview->AddControl(dstPreview);

    zoomSlider = new UISlider(Rect(ControlsFactory::TEXTURE_PREVIEW_WIDTH, rect.dy - ControlsFactory::BUTTON_HEIGHT, 
                                   rect.dx - ControlsFactory::TEXTURE_PREVIEW_WIDTH - ControlsFactory::BUTTON_WIDTH, 
                                   ControlsFactory::BUTTON_HEIGHT));
    zoomSlider->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &TextureConverterDialog::OnZoomChanged));
    zoomSlider->SetMinSprite("~res:/Gfx/LandscapeEditor/Tools/polzunok", 1);
    zoomSlider->SetMinDrawType(UIControlBackground::DRAW_STRETCH_HORIZONTAL);
    zoomSlider->SetMinLeftRightStretchCap(5);
    zoomSlider->SetMaxSprite("~res:/Gfx/LandscapeEditor/Tools/polzunok", 0);
    zoomSlider->SetMaxDrawType(UIControlBackground::DRAW_STRETCH_HORIZONTAL);
    zoomSlider->SetMaxLeftRightStretchCap(5);
    zoomSlider->SetThumbSprite("~res:/Gfx/LandscapeEditor/Tools/polzunokCenter", 0);
    AddControl(zoomSlider);
    
    formatDialog = new TextureFormatDialog(this);
    lastActiveZoomControl = NULL;
}

void TextureConverterDialog::AddLine(const DAVA::Rect &lineRect)
{
    UIControl *line =  ControlsFactory::CreateLine(lineRect, Color(0.2f, 0.2f, 0.2f, 0.8f));
    AddControl(line);
    SafeRelease(line);
}

TextureConverterDialog::~TextureConverterDialog()
{
    SafeRelease(zoomSlider);
    SafeRelease(formatDialog);
    
    SafeRelease(dstZoomPreview);
    SafeRelease(srcZoomPreview);
    SafeRelease(dstPreview);
    SafeRelease(srcPreview);
  
    ReleaseTextures();
    SafeRelease(workingScene);
    
    SafeRelease(convertButton);
    
    SafeRelease(closeButtonTop);
    SafeRelease(textureList);
}

int32 TextureConverterDialog::ElementsCount(UIList * list)
{
    return textures.size();
}

UIListCell *TextureConverterDialog::CellAtIndex(UIList *list, int32 index)
{
    TextureConverterCell *c = (TextureConverterCell *)list->GetReusableCell("TextureConverter cell"); 
    if(!c)
    {
        c = new TextureConverterCell(Rect(0, 0, list->GetSize().dx, 10), "TextureConverter cell");
    }
    
    c->SetSelected(selectedItem == index, false);
    c->SetTexture(GetTextureForIndex(index)->relativePathname);
    return c;
}

int32 TextureConverterDialog::CellHeight(UIList * list, int32 index)
{
    return TextureConverterCell::GetCellHeight();
}

void TextureConverterDialog::Show(Scene * scene)
{
    if(!GetParent())
    {
        SafeRelease(workingScene);
        workingScene = SafeRetain(scene);
        
        selectedTextureName = "";
        EnumerateTextures();
        
        selectedItem = -1;
        textureList->Refresh();
        if(textures.size())
        {
            textureList->ScrollToElement(0);
        }
        
        UIScreen *screen = UIScreenManager::Instance()->GetScreen();
        screen->AddControl(this);
    }
}


void TextureConverterDialog::OnCancel(BaseObject * owner, void * userData, void * callerData)
{
    if(srcZoomPreview->GetParent())
    {
        RemoveControl(srcZoomPreview);
    }
    if(dstZoomPreview->GetParent())
    {
        RemoveControl(dstZoomPreview);
    }

    SafeRelease(workingScene);
    ReleaseTextures();

    if(GetParent())
    {
        GetParent()->RemoveControl(this);
    }
}

void TextureConverterDialog::EnumerateTextures()
{
    if(workingScene)
    {
        EnumerateTexturesFromMaterials();
        EnumerateTexturesFromNodes(workingScene);
    }
}

void TextureConverterDialog::EnumerateTexturesFromMaterials()
{
    Vector<Material *> materials;
    workingScene->GetDataNodes(materials);
    
    for(int32 iMat = 0; iMat < (int32)materials.size(); ++iMat)
    {
        for(int32 iTex = 0; iTex < Material::TEXTURE_COUNT; ++iTex)
        {
			if((materials[iMat]->type == Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP) && iTex > Material::TEXTURE_DIFFUSE)
			{
				continue;
			}

            Texture *t = materials[iMat]->GetTexture((Material::eTextureLevel)iTex);
            CollectTexture(t);
        }
    }
}

void TextureConverterDialog::EnumerateTexturesFromNodes(SceneNode * node)
{
    int32 count = node->GetChildrenCount();
    for(int32 iChild = 0; iChild < count; ++iChild)
    {
        SceneNode *child = node->GetChild(iChild);
        
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*>(child);
        if (landscape) 
        {
            for(int32 iTex = 0; iTex < LandscapeNode::TEXTURE_COUNT; ++iTex)
            {
                Texture *t = landscape->GetTexture((LandscapeNode::eTextureLevel)iTex);
                CollectTexture(t);
            }
        }
        
        EnumerateTexturesFromNodes(child);
    }
}

void TextureConverterDialog::CollectTexture(Texture *texture)
{
    if(texture && !texture->isRenderTarget)
    {
        String::size_type pos = texture->relativePathname.find("~res:/");
        if(String::npos == pos)
        {
//            textures.insert(SafeRetain(texture));
            textures.insert(texture);
        }
    }
}


void TextureConverterDialog::RestoreTextures(Texture *t, const String &newTexturePath)
{
    RestoreTexturesFromMaterials(t, newTexturePath);
    RestoreTexturesFromNodes(t, newTexturePath, workingScene);
}

void TextureConverterDialog::RestoreTexturesFromMaterials(Texture *t, const String &newTexturePath)
{
    Vector<Material *> materials;
    workingScene->GetDataNodes(materials);
    
    for(int32 iMat = 0; iMat < (int32)materials.size(); ++iMat)
    {
        for(int32 iTex = 0; iTex < Material::TEXTURE_COUNT; ++iTex)
        {
            Texture *tex = materials[iMat]->GetTexture((Material::eTextureLevel)iTex);
            if(t == tex)
            {
                materials[iMat]->SetTexture((Material::eTextureLevel)iTex, newTexturePath);
            }
        }
    }

}

void TextureConverterDialog::RestoreTexturesFromNodes(Texture *t, const String &newTexturePath, SceneNode * node)
{
    int32 count = node->GetChildrenCount();
    for(int32 iChild = 0; iChild < count; ++iChild)
    {
        SceneNode *child = node->GetChild(iChild);
        
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*>(child);
        if (landscape) 
        {
            for(int32 iTex = 0; iTex < LandscapeNode::TEXTURE_COUNT; ++iTex)
            {
                Texture *tex = landscape->GetTexture((LandscapeNode::eTextureLevel)iTex);
                if(t == tex)
                {
                    landscape->SetTexture((LandscapeNode::eTextureLevel)iTex, newTexturePath);
                }
            }
        }
        
        RestoreTexturesFromNodes(t, newTexturePath, child);
    }
}


void TextureConverterDialog::OnCellSelected(UIList *forList, UIListCell *selectedCell)
{
    selectedItem = selectedCell->GetIndex();

    SetupTexturePreview();
    
    //set selections
    List<UIControl*> children = forList->GetVisibleCells();
    List<UIControl*>::iterator endIt = children.end();
    for(List<UIControl*>::iterator it = children.begin(); it != endIt; ++it)
    {
        UIControl *ctrl = (*it);
        ctrl->SetSelected(false, false);
    }
    
    selectedCell->SetSelected(true, false);
}

void TextureConverterDialog::OnConvert(DAVA::BaseObject *owner, void *userData, void *callerData)
{
    if(-1 == selectedItem)
    {
        ErrorNotifier::Instance()->ShowError("Texture not selected.");
    }
    else 
    {
        Texture *t = GetTextureForIndex(selectedItem);
        if (t->width != t->height)
        {
            ErrorNotifier::Instance()->ShowError("Wrong size. Texture must be square.");
        }
        else if(!IsPowerOf2(t->width))
        {
            ErrorNotifier::Instance()->ShowError("Wrong size. Size must be power of 2.");
        }
        else 
        {
            formatDialog->Show();
        }
    }
}

void TextureConverterDialog::OnFormatSelected(PixelFormat newFormat, bool generateMimpaps)
{
    Texture *t = GetTextureForIndex(selectedItem);
    
    String newName = PVRConverter::Instance()->ConvertPngToPvr(GetSrcTexturePath(t->relativePathname), newFormat, generateMimpaps);
    RestoreTextures(t, newName);
    
    selectedTextureName = FileSystem::ReplaceExtension(newName, "");

    selectedTextureName = NormalizePath(selectedTextureName);

    ReleaseTextures();
    EnumerateTextures();

    selectedItem = -1;
    Set<Texture *>::iterator it = textures.begin();
    Set<Texture *>::iterator endIt = textures.end();
    
    for(int32 i = 0; it != endIt; ++it, ++i)
    {
        Texture *t = (*it);
        
        String textureName = FileSystem::ReplaceExtension(t->relativePathname, "");
        textureName = NormalizePath(textureName);
        
        if(textureName == selectedTextureName)
        {
            selectedItem = i;
            break;
        }
    }
    
    textureList->Refresh();
    textureList->ScrollToElement(selectedItem);
    
    SetupTexturePreview();
}


Texture *TextureConverterDialog::GetTextureForIndex(int32 index)
{
    Set<Texture *>::iterator it = textures.begin();
    Set<Texture *>::iterator endIt = textures.end();
    
    for(int32 i = 0; it != endIt; ++it, ++i)
    {
        if(index == i)
        {
            return (*it);
        }
    }
    
    return NULL;
}

void TextureConverterDialog::ReleaseTextures()
{
//    Set<Texture *>::iterator it = textures.begin();
//    Set<Texture *>::iterator endIt = textures.end();
//    
//    for(int32 i = 0; it != endIt; ++it, ++i)
//    {
//        Texture *t = (*it);
//        SafeRelease(t);
//    }
    textures.clear();
}

void TextureConverterDialog::SetupTexturePreview()
{
    srcOffsetPrev = Vector2(0, 0);
    dstOffsetPrev = Vector2(0, 0);

    Texture *srcTexture = NULL;
    Texture *dstTexture = NULL;
    
    Texture *workingTexture = GetTextureForIndex(selectedItem);
    String workingTexturePath = workingTexture->relativePathname;

//    bool isEnabled = Image::IsAlphaPremultiplicationEnabled();
//    bool isMipmaps = Texture::IsMipmapGenerationEnabled();
//
//    Image::EnableAlphaPremultiplication(false);
//    Texture::DisableMipmapGeneration();   

    if(FileSystem::GetExtension(workingTexturePath) == ".png")
    {
        srcTexture = Texture::CreateFromFile(workingTexturePath);
        
        String dstPath = FileSystem::ReplaceExtension(workingTexturePath, ".pvr");
        dstTexture = Texture::CreateFromFile(dstPath);
    }
    else if(FileSystem::GetExtension(workingTexturePath) == ".pvr")
    {
        String srcPath = FileSystem::ReplaceExtension(workingTexturePath, ".png");
        srcTexture = Texture::CreateFromFile(srcPath);
        
        dstTexture = Texture::CreateFromFile(workingTexturePath);
    }
    
    if(srcTexture)
    {
        srcTexture->GeneratePixelesation();
    }
    
    if(dstTexture)
    {
        dstTexture->GeneratePixelesation();
    }
    
//    if(isMipmaps)
//    {
//        Texture::EnableMipmapGeneration();
//    }
//    Image::EnableAlphaPremultiplication(isEnabled);
    
    
    SetupZoomedPreview(srcTexture, srcPreview, srcZoomPreview);
    SetupZoomedPreview(dstTexture, dstPreview, dstZoomPreview);
    
    SafeRelease(srcTexture);
    SafeRelease(dstTexture);
}



void TextureConverterDialog::SetupZoomedPreview(Texture *tex, UIControl *preview, UIZoomControl *zoomControl)
{
    if(tex)
    {
        Sprite *sprite = Sprite::CreateFromTexture(tex, 0, 0, (float32)tex->width, (float32)tex->height);
        preview->SetSprite(sprite, 0);
        Vector2 texSize((float32)tex->width, (float32)tex->height);
        preview->SetSize(texSize);
        zoomControl->SetContentSize(texSize);
        zoomControl->SetOffset(Vector2(0, 0));
        float32 minScale = 1.f;
        zoomControl->SetScales(minScale, 10.f);
        zoomControl->SetScale(minScale);
        
        zoomSlider->SetMinMaxValue(minScale, 10.f);
        zoomSlider->SetValue(minScale);

        if(!zoomControl->GetParent())
        {
            AddControl(zoomControl);
        }
    }
    else 
    {
        if(zoomControl->GetParent())
        {
            RemoveControl(zoomControl);
        }
    }
}


void TextureConverterDialog::OnZoomChanged(DAVA::BaseObject *object, void *userData, void *callerData)
{
    float32 scale = zoomSlider->GetValue();
    if(dstZoomPreview->GetParent())
    {
        Vector2 offset = dstZoomPreview->GetOffset();
        if(offset.x < 0 || offset.y < 0)
        {
            Vector2 point = dstZoomPreview->GetSize() / 2;
            Vector2 way = (point - offset);
            way = way / dstZoomPreview->GetScale() * scale;
            offset = point - way;
        }
        
        dstZoomPreview->SetScale(scale);
        dstZoomPreview->SetOffset(offset);
    }
    if(srcZoomPreview->GetParent())
    {
        Vector2 offset = srcZoomPreview->GetOffset();
        if(offset.x < 0 || offset.y < 0)
        {
            Vector2 point = srcZoomPreview->GetSize() / 2;
            Vector2 way = (point - offset);
            way = way / srcZoomPreview->GetScale() * scale;
            offset = point - way;
        }
        
        srcZoomPreview->SetScale(scale);
        srcZoomPreview->SetOffset(offset);
    }
}

void TextureConverterDialog::Update(float32 timeElapsed)
{
    if(srcZoomPreview->GetParent() && dstZoomPreview->GetParent())
    {
        Vector2 srcOffset = srcZoomPreview->GetOffset();
        Vector2 dstOffset = dstZoomPreview->GetOffset();

        if(srcZoomPreview->IsScrolling())
        {
            lastActiveZoomControl = srcZoomPreview;
            
            srcOffsetPrev = srcOffset;
            dstOffsetPrev = srcOffset;
            dstZoomPreview->SetOffset(srcOffset);
        }
        else if(dstZoomPreview->IsScrolling())
        {
            lastActiveZoomControl = dstZoomPreview;
            
            srcOffsetPrev = dstOffset;
            dstOffsetPrev = dstOffset;
            srcZoomPreview->SetOffset(dstOffset);
        }
        else if(lastActiveZoomControl)
        {
            Vector2 offset = lastActiveZoomControl->GetOffset();
            srcZoomPreview->SetOffset(offset);
            dstZoomPreview->SetOffset(offset);
            lastActiveZoomControl = NULL;
        }
    }
    
    UIControl::Update(timeElapsed);
}


String TextureConverterDialog::GetSrcTexturePath(const String &relativeTexturePath)
{
    String textureWorkingPath = FileSystem::ReplaceExtension(relativeTexturePath, ".png");
    return textureWorkingPath;
}

String TextureConverterDialog::NormalizePath(const String &pathname)
{
    Vector<String> tokens;
    Split(pathname, "/", tokens);
    
    String retString = "";
    int32 skipCount = 0;
    for(int32 i = tokens.size() - 1; i >= 0; --i)
    {
        if(tokens[i] == "..")
        {
            ++skipCount;
        }
        else if(skipCount)
        {
            i -= skipCount;
            skipCount = 0;
        }
        else 
        {
            if(i == (int32)tokens.size() - 1)
            {
                retString = tokens[i];
            }
            else 
            {
                retString = tokens[i] + "/" + retString;
            }
        }
    }
    
    if(pathname.length() && '/' == pathname.at(0))
    {
        retString = "/" + retString;
    }
    
    return retString;
}

