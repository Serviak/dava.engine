#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif


// DECLARATIONS
uniform sampler2D texture0;
varying mediump vec2 varTexCoord0;

#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL) 
uniform sampler2D texture1;
varying mediump vec2 varTexCoord1;
#endif 

#if defined(VERTEX_LIT)
uniform lowp vec3 materialDiffuseColor;
uniform lowp vec3 materialSpecularColor;

varying lowp float varDiffuseColor;
varying lowp float varSpecularColor;
#endif 

void main()
{
    // FETCH PHASE
    lowp vec3 textureColor0 = texture2D(texture0, varTexCoord0).rgb;
#if defined(OPAQUE)
    if (textureColor0.a < 0.9)discard;
#endif
    
#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL)
    lowp vec3 textureColor1 = texture2D(texture1, varTexCoord1).rgb;
#endif

    // DRAW PHASE
#if defined(VERTEX_LIT)
	vec3 color = varDiffuseColor * materialDiffuseColor * textureColor0 + varSpecularColor * materialSpecularColor;
#else
    #if defined(MATERIAL_TEXTURE)
    	vec3 color = textureColor0;
    #elif defined(MATERIAL_DECAL)
    	vec3 color = textureColor0 * textureColor1;
    #elif defined(MATERIAL_DETAIL)
        vec3 color = textureColor0 * textureColor1 * 2.0;
    #endif 
#endif
    gl_FragColor = vec4(color, 1.0);
}
