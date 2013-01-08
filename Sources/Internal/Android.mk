#-----------------------------
# Framework lib 

# set local path for lib
LOCAL_PATH := $(call my-dir)

DAVA_ROOT := $(LOCAL_PATH)


# clear all variables
include $(CLEAR_VARS)

# set module name
LOCAL_MODULE := libInternal

# set path for includes
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Libs/include

# set exported includes
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

# set source files 
LOCAL_SRC_FILES :=  \
                    Animation/AnimatedObject.cpp \
                    Animation/Animation.cpp \
                    Animation/AnimationManager.cpp \
                    Animation/BezierSplineAnimation.cpp \
                    Animation/Interpolation.cpp \
                    Animation/KeyframeAnimation.cpp \
                    Animation/LinearAnimation.cpp \
                    \
                    Base/BaseMath.cpp \
                    Base/BaseObject.cpp \
                    Base/BaseObjectChecker.cpp \
                    Base/Data.cpp \
                    Base/DynamicObjectCache.cpp \
                    Base/EventDispatcher.cpp \
                    Base/FastName.cpp \
                    Base/FixedSizePoolAllocator.cpp \
                    Base/Message.cpp \
                    Base/ObjectFactory.cpp \
                    Base/Observable.cpp \
                    \
                    Collision/CollisionObject2.cpp \
                    Collision/CollisionPolygons.cpp \
                    Collision/Collisions.cpp \
                    \
                    Core/ApplicationCore.cpp \
                    Core/Core.cpp \
                    \
                    Database/MongodbClient.cpp \
                    Database/MongodbObject.cpp \
                    \
                    Debug/MemoryManager.cpp \
                    Debug/Stats.cpp \
                    Debug/Replay.cpp \
                    \
                    Entity/Component.cpp \
                    Entity/ComponentTypes.cpp \
                    Entity/Entity.cpp \
                    Entity/EntityFamily.cpp \
                    Entity/EntityManager.cpp \
                    Entity/LandscapeGeometryComponent.cpp \
                    Entity/MeshInstanceComponent.cpp \
                    Entity/MeshInstanceDrawSystem.cpp \
                    Entity/TransformComponent.cpp \
                    Entity/PoolSystem.cpp \
                    Entity/VisibilityAABBoxComponent.cpp \
                    \
                    FileSystem/APKFile.cpp \
                    FileSystem/Bitstream.cpp \
                    FileSystem/DynamicMemoryFile.cpp \
                    FileSystem/File.cpp \
                    FileSystem/FileList.cpp \
                    FileSystem/FileSystem.cpp \
                    FileSystem/KeyedArchive.cpp \
                    FileSystem/KeyedArchiver.cpp \
                    FileSystem/KeyedUnarchiver.cpp \
                    FileSystem/LocalizationSystem.cpp \
                    FileSystem/Logger.cpp \
                    FileSystem/LoggerAndroid.cpp \
                    FileSystem/ResourceArchive.cpp \
                    FileSystem/VariantType.cpp \
                    FileSystem/YamlArchive.cpp \
                    FileSystem/YamlParser.cpp \
                    FileSystem/XMLParser.cpp \
                    \
                    Input/Accelerometer.cpp \
                    Input/AccelerometerAndroid.cpp \
                    Input/InputSystem.cpp \
                    Input/KeyboardDevice.cpp \
                    \
                    Math/Neon/NeonMath.cpp \
                    \
                    Math/AABBox2.cpp \
                    Math/AABBox3.cpp \
                    Math/BezierSpline.cpp \
                    Math/Math2D.cpp \
                    Math/Matrix4.cpp \
                    Math/Polygon2.cpp \
                    Math/Polygon3.cpp \
                    Math/RectPacker.cpp \
                    Math/Spline.cpp \
                    \
                    Network/NetworkConnection.cpp \
                    Network/NetworkDelegate.cpp \
                    Network/NetworkPacket.cpp \
                    \
                    Particles/Particle.cpp \
                    Particles/ParticleEmitter.cpp \
                    Particles/ParticleEmitter3D.cpp \
                    Particles/ParticleEmitterObject.cpp \
                    Particles/ParticleLayer.cpp \
                    Particles/ParticleLayer3D.cpp \
                    Particles/ParticlePropertyLine.cpp \
                    Particles/ParticleSystem.cpp \
                    Particles/ParticleLayerLong.cpp \
                    \
                    Platform/Android/CorePlatformAndroid.cpp \
                    Platform/Android/EGLRenderer.cpp \
                    Platform/Android/ThreadContext.cpp \
                    \
                    Platform/Mutex.cpp \
                    Platform/SystemTimer.cpp \
                    Platform/Thread.cpp \
                    Platform/ThreadAndroid.cpp \
                    \
                    Render/2D/Font.cpp \
                    Render/2D/FontManager.cpp \
                    Render/2D/FTFont.cpp \
                    Render/2D/GraphicsFont.cpp \
                    Render/2D/Sprite.cpp \
                    Render/2D/TextBlock.cpp \
                    \
                    Render/3D/AnimatedMesh.cpp \
                    Render/3D/EdgeAdjacency.cpp \
                    Render/3D/PolygonGroup.cpp \
                    Render/3D/StaticMesh.cpp \
                    \
                    Render/Effects/ColorOnlyEffect.cpp \
                    Render/Effects/MultiTextureEffect.cpp \
                    Render/Effects/TextureMulColorAlphaTestEffect.cpp \
                    Render/Effects/TextureMulColorEffect.cpp \
                    \
                    Render/DynamicIndexBuffer.cpp \
                    Render/DynamicVertexBuffer.cpp \
                    Render/Image.cpp \
                    Render/ImageLoader.cpp \
                    Render/LibDxtHelper.cpp \
                    Render/LibPngHelpers.cpp \
                    Render/LibPVRHelper.cpp \
                    Render/Material.cpp \
                    Render/RenderBase.cpp \
                    Render/RenderDataObject.cpp \
                    Render/RenderEffect.cpp \
                    Render/RenderGrayscaleEffect.cpp \
                    Render/RenderHelper.cpp \
                    Render/RenderManager.cpp \
                    Render/RenderManagerFactory.cpp \
                    Render/RenderManagerGL.cpp \
                    Render/RenderManagerGL20.cpp \
                    Render/RenderOptions.cpp \
                    Render/RenderResource.cpp \
                    Render/RenderStateBlock.cpp \
                    Render/Shader.cpp \
                    Render/ShaderGL.cpp \
                    Render/SharedFBO.cpp \
                    Render/StaticIndexBuffer.cpp \
                    Render/StaticVertexBuffer.cpp \
                    Render/Texture.cpp \
                    Render/TextureDescriptor.cpp \
                    Render/UberShader.cpp \
                    \
                    Scene2D/Box2DDebugDraw.cpp \
                    Scene2D/Box2DGameObject.cpp \
                    Scene2D/Box2DGameObjectManager.cpp \
                    Scene2D/Box2DHelper.cpp \
                    Scene2D/Box2DTileCollider.cpp \
                    Scene2D/GameObject.cpp \
                    Scene2D/GameObjectAnimations.cpp \
                    Scene2D/GameObjectManager.cpp \
                    Scene2D/TextGameObject.cpp \
                    \
                    Scene3D/BillboardNode.cpp \
                    Scene3D/BoneNode.cpp \
                    Scene3D/BVHierarchy.cpp \
                    Scene3D/BVNode.cpp \
                    Scene3D/Camera.cpp \
                    Scene3D/CubeNode.cpp \
                    Scene3D/DataNode.cpp \
                    Scene3D/Heightmap.cpp \
                    Scene3D/Frustum.cpp \
                    Scene3D/ImposterManager.cpp \
                    Scene3D/ImposterNode.cpp \
                    Scene3D/LandscapeCursor.cpp \
                    Scene3D/LandscapeNode.cpp \
                    Scene3D/LightNode.cpp \
                    Scene3D/LodNode.cpp \
                    Scene3D/MeshInstanceNode.cpp \
                    Scene3D/PathManip.cpp \
                    Scene3D/ParticleEffectNode.cpp \
                    Scene3D/ParticleEmitterNode.cpp \
                    Scene3D/ProxyNode.cpp \
                    Scene3D/QuadTree.cpp \
                    Scene3D/ReferenceNode.cpp \
                    Scene3D/RotatingCubeNode.cpp \
                    Scene3D/Scene.cpp \
                    Scene3D/SceneAnimationMixer.cpp \
                    Scene3D/SceneFile.cpp \
                    Scene3D/SceneFileV2.cpp \
                    Scene3D/SceneManager.cpp \
                    Scene3D/SceneNode.cpp \
                    Scene3D/SceneNodeAnimation.cpp \
                    Scene3D/SceneNodeAnimationKey.cpp \
                    Scene3D/SceneNodeAnimationList.cpp \
                    Scene3D/ShadowRect.cpp \
                    Scene3D/ShadowVolumeNode.cpp \
                    Scene3D/SkeletonNode.cpp \
                    Scene3D/SphereNode.cpp \
                    Scene3D/SpriteNode.cpp \
                    Scene3D/SwitchNode.cpp \
                    Scene3D/UserNode.cpp \
                    \
                    Sound/Sound.cpp \
                    Sound/SoundBuffer.cpp \
                    Sound/SoundChannel.cpp \
                    Sound/SoundDataProvider.cpp \
                    Sound/SoundGroup.cpp \
                    Sound/SoundInstance.cpp \
                    Sound/SoundOVProvider.cpp \
                    Sound/SoundSystem.cpp \
                    Sound/SoundWVProvider.cpp \
                    \
                    UI/ScrollHelper.cpp \
                    UI/UI3DView.cpp \
                    UI/UIButton.cpp \
                    UI/UIControl.cpp \
                    UI/UIControlBackground.cpp \
                    UI/UIControlSystem.cpp \
                    UI/UIEvent.cpp \
                    UI/UIFadeTransition.cpp \
                    UI/UIFileSystemDialog.cpp \
                    UI/UIHierarchy.cpp \
                    UI/UIHierarchyCell.cpp \
                    UI/UIHierarchyNode.cpp \
                    UI/UIHoleTransition.cpp \
                    UI/UIJoypad.cpp \
                    UI/UIList.cpp \
                    UI/UIListCell.cpp \
                    UI/UILoadingTransition.cpp \
                    UI/UIMoveInTransition.cpp \
                    UI/UIPopup.cpp \
                    UI/UIScreen.cpp \
                    UI/UIScreenManagerAndroid.cpp \
                    UI/UIScreenTransition.cpp \
                    UI/UIScrollBar.cpp \
                    UI/UISlider.cpp \
                    UI/UIStaticText.cpp \
                    UI/UITextField.cpp \
                    UI/UIYamlLoader.cpp \
                    \
                    Utils/HTTPDownloaderAndroid.cpp \
                    Utils/MD5.cpp \
                    Utils/StringFormat.cpp \
                    Utils/UTF8Utils.cpp \
                    Utils/Utils.cpp \

# set build flags
LOCAL_CFLAGS := -frtti -g -O2 -DGL_GLEXT_PROTOTYPES=1 -Wno-psabi

# set exported build flags
LOCAL_EXPORT_CFLAGS := $(LOCAL_CFLAGS)

# set used libs

LIBS_PATH := $(call host-path,$(LOCAL_PATH)/../../Libs/libs)

LOCAL_LDLIBS := -lGLESv1_CM -llog -lGLESv2 -lEGL
LOCAL_LDLIBS += $(LIBS_PATH)/libxml_android.a
LOCAL_LDLIBS += $(LIBS_PATH)/libpng_android.a
LOCAL_LDLIBS += $(LIBS_PATH)/libfreetype_android.a
LOCAL_LDLIBS += $(LIBS_PATH)/libyaml_android.a
LOCAL_LDLIBS += $(LIBS_PATH)/libmongodb_android.a
LOCAL_LDLIBS += $(LIBS_PATH)/libdxt_android.a
LOCAL_LDLIBS += -fuse-ld=gold -fno-exceptions

# set exported used libs
LOCAL_EXPORT_LDLIBS := $(LOCAL_LDLIBS)

# set arm mode
# LOCAL_ARM_MODE := arm


# set included libraries
LOCAL_STATIC_LIBRARIES := libbox2d
LOCAL_STATIC_LIBRARIES += android_native_app_glue

include $(BUILD_STATIC_LIBRARY)

# include modules
$(call import-add-path,$(DAVA_ROOT)/..)
$(call import-add-path,$(DAVA_ROOT)/../External)
$(call import-add-path,$(DAVA_ROOT)/../External/Box2D)
$(call import-add-path,$(DAVA_ROOT))

$(call import-module,box2d)
$(call import-module,android/native_app_glue)
