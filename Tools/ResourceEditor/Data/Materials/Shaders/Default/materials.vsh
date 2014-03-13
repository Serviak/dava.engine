#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

//#define TEXTURE0_SHIFT_ENABLED

// INPUT ATTRIBUTES
attribute vec4 inPosition;

#if defined(VERTEX_LIT) || defined(PIXEL_LIT) || defined(MATERIAL_GRASS)
attribute vec3 inNormal;
#endif 

#if defined(MATERIAL_SKYBOX)
attribute vec3 inTexCoord0;
#else
attribute vec2 inTexCoord0;
#endif

#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL) || defined(MATERIAL_LIGHTMAP) || defined(FRAME_BLEND)
attribute vec2 inTexCoord1;
#endif

#if defined(VERTEX_COLOR)
attribute vec4 inColor;
#endif

#if defined(MATERIAL_GRASS)
attribute vec3 inBinormal;
#endif

#if defined(VERTEX_LIT)
#endif

#if defined(PIXEL_LIT) || defined(SPEED_TREE_LEAF) || defined(MATERIAL_GRASS)
attribute vec3 inTangent;
#endif

#if defined(FRAME_BLEND)
attribute float inTime;
#endif

// UNIFORMS
uniform mat4 worldViewProjMatrix;

#if defined(VERTEX_LIT) || defined(PIXEL_LIT) || defined(VERTEX_FOG) || defined(SPEED_TREE_LEAF)
uniform mat4 worldViewMatrix;
#endif

#if defined(VERTEX_LIT) || defined(PIXEL_LIT)
uniform mat3 worldViewInvTransposeMatrix;
uniform vec3 lightPosition0;
uniform float lightIntensity0; 
#endif

#if defined(VERTEX_LIT)
uniform float materialSpecularShininess;
#endif

#if defined(VERTEX_FOG)
    #if !defined(FOG_LINEAR)
    uniform float fogDensity;
    #else
    uniform float fogStart;
    uniform float fogEnd;
    #endif
#endif

#if defined(MATERIAL_LIGHTMAP)
uniform mediump vec2 uvOffset;
uniform mediump vec2 uvScale;
#endif

#if defined(SPEED_TREE_LEAF)
uniform vec3 worldViewTranslate;
uniform vec3 worldScale;
uniform mat4 projMatrix;
uniform float cutDistance;
uniform lowp vec3 treeLeafColorMul;
uniform lowp float treeLeafOcclusionOffset;
uniform lowp float treeLeafOcclusionMul;
#endif

// OUTPUT ATTRIBUTES
#if defined(MATERIAL_SKYBOX)
varying vec3 varTexCoord0;
#else
varying vec2 varTexCoord0;
#endif

#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL) || defined(MATERIAL_LIGHTMAP) || defined(FRAME_BLEND) || defined(MATERIAL_GRASS)
varying vec2 varTexCoord1;
#endif

#if defined(VERTEX_LIT)
varying lowp float varDiffuseColor;
varying lowp float varSpecularColor;
#endif

#if defined(PIXEL_LIT)
varying vec3 varLightVec;
varying vec3 varHalfVec;
varying vec3 varEyeVec;
varying float varPerPixelAttenuation;
#endif

#if defined(VERTEX_FOG)
varying float varFogFactor;
#endif

#if defined(SETUP_LIGHTMAP)
uniform float lightmapSize;
varying lowp float varLightmapSize;
#endif

#if defined(VERTEX_COLOR)
varying lowp vec4 varVertexColor;
#endif

#if defined(FRAME_BLEND)
varying lowp float varTime;
#endif

#if defined(TEXTURE0_SHIFT_ENABLED)
uniform mediump vec2 texture0Shift;
#endif 

#if defined(REFLECTION) // works now only with VERTEX_LIT
uniform vec3 cameraPosition;
uniform mat4 worldMatrix;
uniform mat3 worldInvTransposeMatrix;
#if defined(VERTEX_LIT)
varying mediump vec3 reflectionDirectionInWorldSpace;
#elif defined(PIXEL_LIT)
varying mediump vec3 cameraToPointInTangentSpace;
varying mediump mat3 tbnToWorldMatrix;
#endif

#endif

#if defined(TEXTURE0_ANIMATION_SHIFT)
uniform float globalTime;
uniform vec2 tex0ShiftPerSecond;
#endif

#if defined(MATERIAL_GRASS)
uniform vec4 tilePos;
uniform vec3 worldSize;

#if defined(MATERIAL_GRASS_PLAINUNIFORMS)
uniform mat4 clusterScaleDensityMap_0;
uniform mat4 clusterScaleDensityMap_1;
uniform mat4 clusterScaleDensityMap_2;
uniform mat4 clusterScaleDensityMap_3;
uniform mat4 clusterScaleDensityMap_4;
uniform mat4 clusterScaleDensityMap_5;
uniform mat4 clusterScaleDensityMap_6;
uniform mat4 clusterScaleDensityMap_7;
uniform mat4 clusterScaleDensityMap_8;
uniform mat4 clusterScaleDensityMap_9;
uniform mat4 clusterScaleDensityMap_10;
uniform mat4 clusterScaleDensityMap_11;
uniform mat4 clusterScaleDensityMap_12;
uniform mat4 clusterScaleDensityMap_13;
uniform mat4 clusterScaleDensityMap_14;
uniform mat4 clusterScaleDensityMap_15;
#else
uniform mat4 clusterScaleDensityMap[16];
#endif

uniform sampler2D detail;

uniform vec2 heightmapScale;
#endif


void main()
{
#if defined(MATERIAL_SKYBOX)
	vec4 vecPos = (worldViewProjMatrix * inPosition);
	gl_Position = vec4(vecPos.xy, vecPos.w - 0.0001, vecPos.w);
#elif defined(SPEED_TREE_LEAF)

#if defined (CUT_LEAF)
    vec3 position;
    vec4 tangentInCameraSpace = worldViewMatrix * vec4(inTangent, 1);
    if (tangentInCameraSpace.z < -cutDistance)
    {
        position = /*worldScale * vec3(0,0,0) +*/ worldViewTranslate;
    }
    else
    {
        position = worldScale * (inPosition.xyz - inTangent) + worldViewTranslate;
    }
    gl_Position = projMatrix * vec4(position, inPosition.w) + worldViewProjMatrix * vec4(inTangent, 0.0);
#else
    //mat4 mvp = worldMatrix * viewMatrix * projMatrix;
    //mat4 mvp = projMatrix * worldViewMatrix;
    //gl_Position = mvp * inPosition;
    
//    vec3 position;
//    vec3 tangentInCameraSpace = (worldScale * inTangent + worldViewTranslate);
//    float distance = length(tangentInCameraSpace);
//    if (distance > 40.0)
//    {
//        position = worldScale * ((inPosition.xyz - inTangent) * (40.0 / distance)) + worldViewTranslate;
//    }
//    else
//    {
//        position = worldScale * (inPosition.xyz - inTangent) + worldViewTranslate;
//    }
//    
//    //    vec4 position = vec4(worldScale * (inPosition.xyz - inTangent) + worldViewTranslate, inPosition.w);
//    gl_Position = projMatrix * vec4(position, inPosition.w) + worldViewProjMatrix * vec4(inTangent, 0.0);
    
	gl_Position = projMatrix * vec4(worldScale * (inPosition.xyz - inTangent) + worldViewTranslate, inPosition.w) + worldViewProjMatrix * vec4(inTangent, 0.0);
#endif
#else
    
    #if defined(MATERIAL_GRASS)
    
        //inTangent.y - cluster type (0...3)
        //inTangent.z - cluster's reference density (0...15)
    
        //clusterScaleDensityMap[0] - density
        //clusterScaleDensityMap[1] - scale
    
        vec4 clusterCenter = vec4(inBinormal.x + tilePos.x,
                                  inBinormal.y + tilePos.y,
                                  inBinormal.z,
                                  inPosition.w);
    
        vec4 pos = vec4(inPosition.x + tilePos.x,
                        inPosition.y + tilePos.y,
                        inPosition.z,
                        inPosition.w);
    
        highp vec2 hUV = vec2(clamp(1.0 - (0.5 * worldSize.x - pos.x) / worldSize.x, 0.0, 1.0),
                        clamp(1.0 - (0.5 * worldSize.y - pos.y) / worldSize.y, 0.0, 1.0));
    
        hUV = vec2(clamp(hUV.x * heightmapScale.x, 0.0, 1.0),
                   clamp(hUV.y * heightmapScale.y, 0.0, 1.0));
    
        highp vec4 heightVec = texture2DLod(detail, hUV, 0.0);
        //highp vec4 wVec = vec4(16.0*16.0*16.0*15.0/65535.0, 16.0*16.0*15.0/65535.0, 16.0*15.0/65535.0, 15.0/65535.0);
        //highp vec4 wVec = vec4(0.93751430533303, 0.05859464408331, 0.00366216525521, 0.00022888532845);
        float height = dot(heightVec, vec4(0.93751430533303, 0.05859464408331, 0.00366216525521, 0.00022888532845)) * worldSize.z;
    
        pos.z += height;
        clusterCenter.z += height;
    
        int clusterType = int(inTangent.y);
        int vertexTileIndex = int(inTangent.x);

    
        #if defined(MATERIAL_GRASS_PLAINUNIFORMS)
    
            float clusterScale = 1.0;
            float clusterDensity = 0.0;
    
            if(0 == vertexTileIndex)
            {
                if(0 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_0[1][0];
                    clusterDensity = clusterScaleDensityMap_0[0][0];
                }
                else if(1 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_0[1][1];
                    clusterDensity = clusterScaleDensityMap_0[0][1];
                }
                else if(2 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_0[1][2];
                    clusterDensity = clusterScaleDensityMap_0[0][2];
                }
                else if(3 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_0[1][3];
                    clusterDensity = clusterScaleDensityMap_0[0][3];
                }
            }
            else if(1 == vertexTileIndex)
            {
                if(0 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_1[1][0];
                    clusterDensity = clusterScaleDensityMap_1[0][0];
                }
                else if(1 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_1[1][1];
                    clusterDensity = clusterScaleDensityMap_1[0][1];
                }
                else if(2 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_1[1][2];
                    clusterDensity = clusterScaleDensityMap_1[0][2];
                }
                else if(3 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_1[1][3];
                    clusterDensity = clusterScaleDensityMap_1[0][3];
                }

            }
            else if(2 == vertexTileIndex)
            {
                if(0 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_2[1][0];
                    clusterDensity = clusterScaleDensityMap_2[0][0];
                }
                else if(1 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_2[1][1];
                    clusterDensity = clusterScaleDensityMap_2[0][1];
                }
                else if(2 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_2[1][2];
                    clusterDensity = clusterScaleDensityMap_2[0][2];
                }
                else if(3 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_2[1][3];
                    clusterDensity = clusterScaleDensityMap_2[0][3];
                }

            }
            else if(3 == vertexTileIndex)
            {
                if(0 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_3[1][0];
                    clusterDensity = clusterScaleDensityMap_3[0][0];
                }
                else if(1 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_3[1][1];
                    clusterDensity = clusterScaleDensityMap_3[0][1];
                }
                else if(2 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_3[1][2];
                    clusterDensity = clusterScaleDensityMap_3[0][2];
                }
                else if(3 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_3[1][3];
                    clusterDensity = clusterScaleDensityMap_3[0][3];
                }
            }
            else if(4 == vertexTileIndex)
            {
                if(0 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_4[1][0];
                    clusterDensity = clusterScaleDensityMap_4[0][0];
                }
                else if(1 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_4[1][1];
                    clusterDensity = clusterScaleDensityMap_4[0][1];
                }
                else if(2 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_4[1][2];
                    clusterDensity = clusterScaleDensityMap_4[0][2];
                }
                else if(3 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_4[1][3];
                    clusterDensity = clusterScaleDensityMap_4[0][3];
                }
            }
            else if(5 == vertexTileIndex)
            {
                if(0 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_5[1][0];
                    clusterDensity = clusterScaleDensityMap_5[0][0];
                }
                else if(1 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_5[1][1];
                    clusterDensity = clusterScaleDensityMap_5[0][1];
                }
                else if(2 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_5[1][2];
                    clusterDensity = clusterScaleDensityMap_5[0][2];
                }
                else if(3 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_5[1][3];
                    clusterDensity = clusterScaleDensityMap_5[0][3];
                }
            }
            else if(6 == vertexTileIndex)
            {
                if(0 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_6[1][0];
                    clusterDensity = clusterScaleDensityMap_6[0][0];
                }
                else if(1 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_6[1][1];
                    clusterDensity = clusterScaleDensityMap_6[0][1];
                }
                else if(2 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_6[1][2];
                    clusterDensity = clusterScaleDensityMap_6[0][2];
                }
                else if(3 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_6[1][3];
                    clusterDensity = clusterScaleDensityMap_6[0][3];
                }
            }
            else if(7 == vertexTileIndex)
            {
                if(0 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_7[1][0];
                    clusterDensity = clusterScaleDensityMap_7[0][0];
                }
                else if(1 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_7[1][1];
                    clusterDensity = clusterScaleDensityMap_7[0][1];
                }
                else if(2 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_7[1][2];
                    clusterDensity = clusterScaleDensityMap_7[0][2];
                }
                else if(3 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_7[1][3];
                    clusterDensity = clusterScaleDensityMap_7[0][3];
                }
            }
            else if(8 == vertexTileIndex)
            {
                if(0 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_8[1][0];
                    clusterDensity = clusterScaleDensityMap_8[0][0];
                }
                else if(1 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_8[1][1];
                    clusterDensity = clusterScaleDensityMap_8[0][1];
                }
                else if(2 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_8[1][2];
                    clusterDensity = clusterScaleDensityMap_8[0][2];
                }
                else if(3 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_8[1][3];
                    clusterDensity = clusterScaleDensityMap_8[0][3];
                }
            }
            else if(9 == vertexTileIndex)
            {
                if(0 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_9[1][0];
                    clusterDensity = clusterScaleDensityMap_9[0][0];
                }
                else if(1 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_9[1][1];
                    clusterDensity = clusterScaleDensityMap_9[0][1];
                }
                else if(2 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_9[1][2];
                    clusterDensity = clusterScaleDensityMap_9[0][2];
                }
                else if(3 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_9[1][3];
                    clusterDensity = clusterScaleDensityMap_9[0][3];
                }
            }
            else if(10 == vertexTileIndex)
            {
                if(0 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_10[1][0];
                    clusterDensity = clusterScaleDensityMap_10[0][0];
                }
                else if(1 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_10[1][1];
                    clusterDensity = clusterScaleDensityMap_10[0][1];
                }
                else if(2 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_10[1][2];
                    clusterDensity = clusterScaleDensityMap_10[0][2];
                }
                else if(3 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_10[1][3];
                    clusterDensity = clusterScaleDensityMap_10[0][3];
                }
            }
            else if(11 == vertexTileIndex)
            {
                if(0 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_11[1][0];
                    clusterDensity = clusterScaleDensityMap_11[0][0];
                }
                else if(1 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_11[1][1];
                    clusterDensity = clusterScaleDensityMap_11[0][1];
                }
                else if(2 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_11[1][2];
                    clusterDensity = clusterScaleDensityMap_11[0][2];
                }
                else if(3 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_11[1][3];
                    clusterDensity = clusterScaleDensityMap_11[0][3];
                }
            }
            else if(12 == vertexTileIndex)
            {
                if(0 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_12[1][0];
                    clusterDensity = clusterScaleDensityMap_12[0][0];
                }
                else if(1 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_12[1][1];
                    clusterDensity = clusterScaleDensityMap_12[0][1];
                }
                else if(2 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_12[1][2];
                    clusterDensity = clusterScaleDensityMap_12[0][2];
                }
                else if(3 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_12[1][3];
                    clusterDensity = clusterScaleDensityMap_12[0][3];
                }
            }
            else if(13 == vertexTileIndex)
            {
                if(0 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_13[1][0];
                    clusterDensity = clusterScaleDensityMap_13[0][0];
                }
                else if(1 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_13[1][1];
                    clusterDensity = clusterScaleDensityMap_13[0][1];
                }
                else if(2 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_13[1][2];
                    clusterDensity = clusterScaleDensityMap_13[0][2];
                }
                else if(3 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_13[1][3];
                    clusterDensity = clusterScaleDensityMap_13[0][3];
                }
            }
            else if(14 == vertexTileIndex)
            {
                if(0 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_14[1][0];
                    clusterDensity = clusterScaleDensityMap_14[0][0];
                }
                else if(1 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_14[1][1];
                    clusterDensity = clusterScaleDensityMap_14[0][1];
                }
                else if(2 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_14[1][2];
                    clusterDensity = clusterScaleDensityMap_14[0][2];
                }
                else if(3 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_14[1][3];
                    clusterDensity = clusterScaleDensityMap_14[0][3];
                }
            }
            else if(15 == vertexTileIndex)
            {
                if(0 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_15[1][0];
                    clusterDensity = clusterScaleDensityMap_15[0][0];
                }
                else if(1 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_15[1][1];
                    clusterDensity = clusterScaleDensityMap_15[0][1];
                }
                else if(2 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_15[1][2];
                    clusterDensity = clusterScaleDensityMap_15[0][2];
                }
                else if(3 == clusterType)
                {
                    clusterScale = clusterScaleDensityMap_15[1][3];
                    clusterDensity = clusterScaleDensityMap_15[0][3];
                }
            }
    
            pos = mix(clusterCenter, pos, clusterScale * step(inTangent.z, clusterDensity));
    
        #else
    
            pos = mix(clusterCenter, pos, clusterScaleDensityMap[vertexTileIndex][1][clusterType] * step(inTangent.z, clusterScaleDensityMap[vertexTileIndex][0][clusterType]));
    
        #endif
    
        gl_Position = worldViewProjMatrix * pos;
        varTexCoord1 = hUV;
    
    #else
        gl_Position = worldViewProjMatrix * inPosition;
    #endif
    
#endif
#if defined(VERTEX_LIT)
    vec3 eyeCoordsPosition = vec3(worldViewMatrix * inPosition); // view direction in view space
    vec3 normal = normalize(worldViewInvTransposeMatrix * inNormal); // normal in eye coordinates
    vec3 lightDir = lightPosition0 - eyeCoordsPosition;
    
#if defined(DISTANCE_ATTENUATION)
    float attenuation = lightIntensity0;
    float distAttenuation = length(lightDir);
    attenuation /= (distAttenuation * distAttenuation); // use inverse distance for distance attenuation
#endif
    lightDir = normalize(lightDir);
    
#if defined(REFLECTION)
    vec3 viewDirectionInWorldSpace = vec3(worldMatrix * inPosition) - cameraPosition;
    vec3 normalDirectionInWorldSpace = normalize(vec3(worldInvTransposeMatrix * inNormal));
    reflectionDirectionInWorldSpace = reflect(viewDirectionInWorldSpace, normalDirectionInWorldSpace);
#endif
    
    varDiffuseColor = max(0.0, dot(normal, lightDir));

    // Blinn-phong reflection
    vec3 E = normalize(-eyeCoordsPosition);
    vec3 H = normalize(lightDir + E);
    float nDotHV = max(0.0, dot(normal, H));
    
    /*
        Phong Reflection
        vec3 E = normalize(-eyeCoordsPosition);
        vec3 L = lightDir;
        vec3 R = reflect(-L, normal);
        float nDotHV = max(0.0, dot(E, R));
    */
    
    varSpecularColor = pow(nDotHV, materialSpecularShininess);
#endif

#if defined(PIXEL_LIT)
	vec3 n = normalize (worldViewInvTransposeMatrix * inNormal);
	vec3 t = normalize (worldViewInvTransposeMatrix * inTangent);
	vec3 b = cross (n, t);

    vec3 eyeCoordsPosition = vec3(worldViewMatrix *  inPosition);
    
    vec3 lightDir = lightPosition0 - eyeCoordsPosition;
    varPerPixelAttenuation = length(lightDir);
    lightDir = normalize(lightDir);
    
	// transform light and half angle vectors by tangent basis
	vec3 v;
	v.x = dot (lightDir, t);
	v.y = dot (lightDir, b);
	v.z = dot (lightDir, n);
	varLightVec = normalize (v);

    // eyeCoordsPosition = -eyeCoordsPosition;
	// v.x = dot (eyeCoordsPosition, t);
	// v.y = dot (eyeCoordsPosition, b);
	// v.z = dot (eyeCoordsPosition, n);
	// varEyeVec = normalize (v);

    vec3 E = normalize(-eyeCoordsPosition);

	/* Normalize the halfVector to pass it to the fragment shader */

	// No need to divide by two, the result is normalized anyway.
	// vec3 halfVector = normalize((E + lightDir) / 2.0); 
	vec3 halfVector = normalize(E + lightDir);
	v.x = dot (halfVector, t);
	v.y = dot (halfVector, b);
	v.z = dot (halfVector, n);

	// No need to normalize, t,b,n and halfVector are normal vectors.
	//normalize (v);
	varHalfVec = v;
    
#if defined(REFLECTION)
    v.x = dot (eyeCoordsPosition, t);
	v.y = dot (eyeCoordsPosition, b);
	v.z = dot (eyeCoordsPosition, n);
	cameraToPointInTangentSpace = normalize (v);
    
    vec3 binormTS = cross(inNormal, inTangent);
//    tbnToWorldMatrix = mat3(vec3(inTangent.x, binormTS.x, inNormal.x),
//                            vec3(inTangent.y, binormTS.y, inNormal.y),
//                            vec3(inTangent.z, binormTS.z, inNormal.z));
    tbnToWorldMatrix = mat3(inTangent, binormTS, inNormal);
#endif
#endif

#if defined(VERTEX_FOG)
    #if defined(VERTEX_LIT) || defined(PIXEL_LIT)
        float fogFragCoord = length(eyeCoordsPosition);
    #else
        vec3 eyeCoordsPosition = vec3(worldViewMatrix * inPosition);
        float fogFragCoord = length(eyeCoordsPosition);
    #endif
    #if !defined(FOG_LINEAR)
        const float LOG2 = 1.442695;
        varFogFactor = exp2( -fogDensity * fogDensity * fogFragCoord * fogFragCoord *  LOG2);
        varFogFactor = clamp(varFogFactor, 0.0, 1.0);
    #else
        varFogFactor = 1.0 - clamp((fogFragCoord - fogStart) / (fogEnd - fogStart), 0.0, 1.0);
    #endif
	//varFogFactor = 1.0;
#endif

#if defined(VERTEX_COLOR)
	varVertexColor = inColor;
#endif

	varTexCoord0 = inTexCoord0;
	
#if defined(TEXTURE0_SHIFT_ENABLED)
	varTexCoord0 += texture0Shift;
#endif
    
#if defined(TEXTURE0_ANIMATION_SHIFT)
    varTexCoord0 += tex0ShiftPerSecond * globalTime;
#endif
		
#if defined(MATERIAL_DECAL) || defined(MATERIAL_DETAIL) || defined(MATERIAL_LIGHTMAP) || defined(FRAME_BLEND)
	
	#if defined(SETUP_LIGHTMAP)
		varLightmapSize = lightmapSize;
		varTexCoord1 = inTexCoord1;
	#elif defined(MATERIAL_LIGHTMAP)
		varTexCoord1 = uvScale*inTexCoord1+uvOffset;
    #else
		varTexCoord1 = inTexCoord1;
	#endif
#endif

#if defined(FRAME_BLEND)
	varTime = inTime;
#endif

}
