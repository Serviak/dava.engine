#ifndef __DAVAENGINE_TEXTBLOCK_RENDER_H__
#define __DAVAENGINE_TEXTBLOCK_RENDER_H__

#include "Render/2D/TextBlock.h"
#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"

namespace DAVA
{
class TextBlockRender : public BaseObject
{
public:
    TextBlockRender(TextBlock*);
    virtual ~TextBlockRender();

    virtual void Prepare();
    virtual void PreDraw(){};
    virtual void Draw(const Color& /*textColor*/, const Vector2* /*offset*/){};

    Sprite* GetSprite() const
    {
        return sprite;
    };

protected:
    void DrawText();
    virtual Font::StringMetrics DrawTextSL(const WideString& drawText, int32 x, int32 y, int32 w) = 0;
    virtual Font::StringMetrics DrawTextML(const WideString& drawText,
                                           int32 x, int32 y, int32 w,
                                           int32 xOffset, uint32 yOffset,
                                           int32 lineSize) = 0;

protected:
    TextBlock* textBlock;
    Sprite* sprite;
};

}; //end of namespace

#endif // __DAVAENGINE_TEXTBLOCK_RENDER_H__