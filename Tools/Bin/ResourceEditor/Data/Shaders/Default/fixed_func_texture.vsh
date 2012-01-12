#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif


attribute vec4 inPosition;
attribute vec2 inTexCoord;

uniform mat4 modelViewProjectionMatrix;
uniform lowp vec4 flatColor;
varying lowp vec4 varColor;
varying mediump vec2 varTexCoord;

void main()
{
	gl_Position = modelViewProjectionMatrix * inPosition;
	varColor = flatColor * flatColor.a;
	varTexCoord = inTexCoord;
}
