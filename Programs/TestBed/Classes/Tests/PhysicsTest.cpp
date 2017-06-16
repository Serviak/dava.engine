#include "Tests/PhysicsTest.h"
#include "TestBed.h"

#include <Physics/PhysicsModule.h>
#include <Physics/Private/PhysicsMath.h>
#include <Physics/PhysicsConfigs.h>

#include <UI/Update/UIUpdateComponent.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <ModuleManager/ModuleManager.h>

#include <physx/PxScene.h>
#include <physx/PxPhysics.h>
#include <physx/PxRigidActor.h>
#include <physx/PxActor.h>
#include <physx/pvd/PxPvdSceneClient.h>
#include <PxShared/foundation/PxMat44.h>
#include <PxShared/foundation/PxTransform.h>

PhysicsTest::PhysicsTest(TestBed& app)
    : BaseScreen(app, "PhysicsTest")
{
}

void PhysicsTest::LoadResources()
{
    GetOrCreateComponent<DAVA::UIUpdateComponent>();

    BaseScreen::LoadResources();
    const DAVA::EngineContext* ctx = app.GetEngine().GetContext();
    DAVA::Physics* physicsModule = ctx->moduleManager->GetModule<DAVA::Physics>();

    scene = physicsModule->CreateScene(DAVA::PhysicsSceneConfig());
    physx::PxActor* actor = physicsModule->CreateStaticActor();
    physx::PxRigidStatic* staticActor = actor->is<physx::PxRigidStatic>();
    physx::PxShape* shape = physicsModule->CreateBoxShape(true);

    staticActor->attachShape(*shape);
    shape->release();

    scene->addActor(*actor);
    simulationBlock = physicsModule->Allocate(simulationBlockSize, "SimulationBlock", __FILE__, __LINE__);
}

void PhysicsTest::UnloadResources()
{
    const DAVA::EngineContext* ctx = app.GetEngine().GetContext();
    DAVA::Physics* physicsModule = ctx->moduleManager->GetModule<DAVA::Physics>();

    physicsModule->Deallocate(simulationBlock);
    scene->release();
    scene = nullptr;
    BaseScreen::UnloadResources();
}

void PhysicsTest::Update(DAVA::float32 timeElapsed)
{
    scene->simulate(timeElapsed, nullptr, simulationBlock, simulationBlockSize, true);
    physx::PxU32 errorState = 0;
    scene->fetchResults(true, &errorState);
    physx::PxPvdSceneClient* client = scene->getScenePvdClient();
    if (client != nullptr)
    {
        client->updateCamera(nullptr, physx::PxVec3(10.0f, 10.0f, 10.0f), physx::PxVec3(0.0f, 0.0f, 1.0f), physx::PxVec3(0.0f, 0.0f, 0.0f));
    }
}
