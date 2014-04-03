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


#include "JPEGTest.h"
#include "TextureUtils.h"

static const PixelFormat formats[] =
{
	FORMAT_RGBA8888,
	FORMAT_A16
};

JPEGTest::JPEGTest()
: TestTemplate<JPEGTest>("JPEGTest")
{
    FilePath testFolder = FileSystem::Instance()->GetCurrentDocumentsDirectory() + "JPEGTest/JPEG";
    FileSystem::Instance()->CreateDirectory(testFolder, true);
    
    pngSprite = NULL;
    jpegSprite = NULL;
   // decompressedPNGSprite = NULL;

    currentTest = FIRST_TEST;
    for(int32 i = 0; i < TESTS_COUNT; ++i)
    {
        PixelFormatDescriptor formatDescriptor = Texture::GetPixelFormatDescriptor(formats[i]);
        RegisterFunction(this, &JPEGTest::TestFunction, Format("JPEGTest of %s", formatDescriptor.name.c_str()), NULL);
    }
}

void JPEGTest::LoadResources()
{
    GetBackground()->SetColor(Color(0.0f, 1.0f, 0.0f, 1.0f));

    Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);

    font->SetSize(20);

    compareResultText = new UIStaticText(Rect(0, 256, 512, 200));
    compareResultText->SetAlign(ALIGN_LEFT | ALIGN_VCENTER);
    compareResultText->SetMultiline(true);
    compareResultText->SetFont(font);
    compareResultText->SetTextColor(Color::White);
    AddControl(compareResultText);

    SafeRelease(font);
}


void JPEGTest::UnloadResources()
{
    RemoveAllControls();

    SafeRelease(compareResultText);

    SafeRelease(pngSprite);
    SafeRelease(jpegSprite);
    //SafeRelease(decompressedPNGSprite);
}

void JPEGTest::TestFunction(PerfFuncData * data)
{
    DVASSERT(currentTest < TESTS_COUNT);
    
    ReloadSprites();

	float32 differencePersentage = 100.f;
	if (pngSprite->GetSize() == jpegSprite->GetSize())
	{
		TextureUtils::CompareResult result = TextureUtils::CompareSprites(pngSprite, jpegSprite, formats[currentTest]);
		differencePersentage = ((float32)result.difference / ((float32)result.bytesCount * 256.f)) * 100.f;
	}
    
    PixelFormatDescriptor formatDescriptor = Texture::GetPixelFormatDescriptor(formats[currentTest]);
    data->testData.message = Format("\nDifference: %f%%\nCoincidence: %f%%",
                                    differencePersentage, 100.f - differencePersentage);

    compareResultText->SetText(StringToWString(data->testData.message));
    Logger::Debug(data->testData.message.c_str());

    TEST_VERIFY(differencePersentage < (float32)ACCETABLE_DELTA_IN_PERSENTS);

    //Save images for visual comparision
    Image *firstComparer = TextureUtils::CreateImageAsRGBA8888(pngSprite);
    Image *secondComparer = TextureUtils::CreateImageAsRGBA8888(jpegSprite);
    
    FilePath documentsPath = FileSystem::Instance()->GetCurrentDocumentsDirectory();
    ImageLoader::Save(firstComparer, documentsPath + (Format("JPEGTest/src_number_%d.png", currentTest)));
    ImageLoader::Save(secondComparer, documentsPath + (Format("JPEGTest/dst_number_%d.png", currentTest)));
    

    ++currentTest;
}

void JPEGTest::ReloadSprites()
{
    SafeRelease(pngSprite);
    SafeRelease(jpegSprite);

    FilePath path =FilePath::FilepathInDocuments(Format("/JPEGTest/JPEG/number_%d.jpeg", currentTest));
    
    pngSprite = TextureUtils::CreateSpriteFromTexture(String(Format("~res:/TestData/JPEGTest/PNG/number_%d.png", currentTest)));
    
    Image* img = pngSprite->GetTexture()->CreateImageFromMemory(RenderState::RENDERSTATE_2D_BLEND);
    ImageLoader::Save(img, path);
    
    Vector<Image *> imageSet;
	ImageLoader::CreateFromFileByContent( path, imageSet);
    
    DVASSERT(imageSet.size());
    Image* imgJpeg = imageSet[0];
    Texture* tex = Texture::CreateFromData(imgJpeg->GetPixelFormat(), imgJpeg->data, imgJpeg->width, imgJpeg->height, false);
    jpegSprite = Sprite::CreateFromTexture(tex, 0, 0, tex->width, tex->width);
}

void JPEGTest::Draw(const DAVA::UIGeometricData &geometricData)
{
    RenderManager::Instance()->ClearWithColor(0.f, 0.0f, 0.f, 1.f);

    Sprite::DrawState state;
    state.SetFrame(0);

    if(pngSprite)
    {
        state.SetPosition(0.f, 0.f);
        state.SetScaleSize(256.f, 256.f, pngSprite->GetWidth(), pngSprite->GetHeight());
        pngSprite->Draw(&state);
    }

    if(jpegSprite)
    {
        state.SetPosition(260.f, 0.f);
        state.SetScaleSize(256.f, 256.f, jpegSprite->GetWidth(), jpegSprite->GetHeight());
        jpegSprite->Draw(&state);
    }

    TestTemplate<JPEGTest>::Draw(geometricData);
}


