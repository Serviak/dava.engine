#include "TextureConverterCell.h"
#include "ControlsFactory.h"

#include "Render/LibPVRHelper.h"
#include "../Qt/Main/QtUtils.h"

TextureConverterCell::TextureConverterCell(const Rect &rect, const String &cellIdentifier)
:   UIListCell(Rect(rect.x, rect.y, rect.dx, (float32)GetCellHeight()), cellIdentifier)
{
    ControlsFactory::CustomizeListCell(this, L"");
    
    textureName = new UIStaticText(Rect(0, 0, rect.dx, ControlsFactory::BUTTON_HEIGHT));
    textureName->SetFont(ControlsFactory::GetFontDark());
    textureName->SetInputEnabled(false);
    AddControl(textureName);
    
    preview = new UIControl(Rect(0, textureName->GetRect().y + textureName->GetRect().dy, 
                                 rect.dx, ControlsFactory::TEXTURE_PREVIEW_HEIGHT));
    preview->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_PROPORTIONAL);
    preview->SetInputEnabled(false);
    AddControl(preview);
    

    textureFormat = new UIStaticText(Rect(0, preview->GetRect().y + preview->GetRect().dy, 
                                          rect.dx, ControlsFactory::BUTTON_HEIGHT));
    textureFormat->SetFont(ControlsFactory::GetFontDark());
    textureFormat->SetInputEnabled(false);
    AddControl(textureFormat);
    
    textureDimensions = new UIStaticText(Rect(0, textureFormat->GetRect().y + textureFormat->GetRect().dy, 
                                          rect.dx, ControlsFactory::BUTTON_HEIGHT));
    textureDimensions->SetFont(ControlsFactory::GetFontDark());
    textureDimensions->SetInputEnabled(false);
    AddControl(textureDimensions);

    textureSize = new UIStaticText(Rect(0, textureDimensions->GetRect().y + textureDimensions->GetRect().dy, 
                                              rect.dx, ControlsFactory::BUTTON_HEIGHT));
    textureSize->SetFont(ControlsFactory::GetFontDark());
    textureSize->SetInputEnabled(false);
    AddControl(textureSize);

    
    UIControl *line =  ControlsFactory::CreateLine(Rect(0, (float32)GetCellHeight() - 1.f, rect.dx, 1.f), Color(0.2f, 0.2f, 0.2f, 0.8f));
    AddControl(line);
    SafeRelease(line);
}

TextureConverterCell::~TextureConverterCell()
{
    SafeRelease(preview);
    SafeRelease(textureName);
    SafeRelease(textureFormat);
    SafeRelease(textureDimensions);
    SafeRelease(textureSize);
}

void TextureConverterCell::SetTexture(const String &texturePath)
{
    String textureWorkingPath = texturePath;
    
    String path, filename;
    FileSystem::SplitPath(textureWorkingPath, path, filename);

    textureFormat->SetText(L"");
    textureSize->SetText(L"");
    textureName->SetText(StringToWString(filename));
    
    Texture *texture = Texture::CreateFromFile(textureWorkingPath);
    Sprite *s = Sprite::CreateFromTexture(texture, 0, 0, (float32)texture->width, (float32)texture->height);
    preview->SetSprite(s, 0);
    
    String ext = FileSystem::GetExtension(filename);
    if(".png" == ext)
    {
        String pngFormat = Texture::GetPixelFormatString(texture->format);
        
        String pvrPath = FileSystem::ReplaceExtension(textureWorkingPath, ".pvr");
        Texture *pvrTex = Texture::CreateFromFile(pvrPath);
        if(pvrTex)
        {
            PixelFormat format = LibPVRHelper::GetPixelFormat(pvrPath);
            uint32 pvrDataSize = LibPVRHelper::GetDataLength(pvrPath);

            String pvrFormat = Texture::GetPixelFormatString(format);
            textureFormat->SetText(StringToWString(pngFormat + "/" + pvrFormat));
            
            textureSize->SetText(SizeInBytesToWideString(pvrDataSize));
            
            SafeRelease(pvrTex);
        }
        else 
        {
            textureFormat->SetText(StringToWString(pngFormat));
        }
    }
    else if(".pvr" == ext)
    {
        PixelFormat format = LibPVRHelper::GetPixelFormat(textureWorkingPath);
        uint32 pvrDataSize = LibPVRHelper::GetDataLength(textureWorkingPath);

        String pvrFormat = Texture::GetPixelFormatString(format);
        textureSize->SetText(SizeInBytesToWideString(pvrDataSize));

        String pngPath = FileSystem::ReplaceExtension(textureWorkingPath, ".pvr");
        Texture *pngTex = Texture::CreateFromFile(pngPath);
        if(pngTex)
        {
            String pngFormat = Texture::GetPixelFormatString(pngTex->format);
            textureFormat->SetText(StringToWString(pngFormat + "/" + pvrFormat));
            
            SafeRelease(pngTex);
        }
        else 
        {
            textureFormat->SetText(StringToWString(pvrFormat));
        }
    }

    textureDimensions->SetText(Format(L"%d x %d", texture->width, texture->height));
    
    SafeRelease(texture);
    SafeRelease(s);
}


int32 TextureConverterCell::GetCellHeight()
{
    return (ControlsFactory::BUTTON_HEIGHT * 4 + ControlsFactory::TEXTURE_PREVIEW_HEIGHT + 1);
}

