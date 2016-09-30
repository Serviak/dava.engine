#include "Tests/ScriptingTest.h"
#include "Base/Type.h"
#include "Reflection/Registrator.h"
#include "Scripting/LuaScript.h"

using namespace DAVA;

class SubObj : public ReflectedBase
{
    DAVA_VIRTUAL_REFLECTION(SubObj)
    {
        ReflectionRegistrator<SubObj>::Begin()
        .Field("a", &SubObj::a)
        .Field("b", &SubObj::b)
        .Field("c", &SubObj::c)
        .End();
    }

public:
    int32 a = 10;
    WideString b = L"WideString";
    Color c = Color::Black;
};

class DemoObj : public ReflectedBase
{
    DAVA_VIRTUAL_REFLECTION(DemoObj)
    {
        ReflectionRegistrator<DemoObj>::Begin()
        .Field("a", &DemoObj::a)
        .Field("b", &DemoObj::b)
        .Field("c", &DemoObj::c)
        .Field("d", &DemoObj::d)
        .Field("v", &DemoObj::v)
        .End();
    }

public:
    int32 a = 99;
    String b = "String";
    Color c = Color::White;
    SubObj d;
    Vector<int32> v;
};

static const String demo_script = R"script(

function main(context)
    DV.Debug("GlobObj.a: "..tostring(GlobObj.a))
    
    DV.Debug("context: "..tostring(context))
    DV.Debug("context.a: "..tostring(context.a))
    DV.Debug("context[\"a\"]: "..tostring(context["a"]))
    DV.Debug("context.b: "..tostring(context.b))
    DV.Debug("context.c: "..tostring(context.c))
    DV.Debug("context.d: "..tostring(context.d))
    DV.Debug("context.d.a: "..tostring(context.d.a))
    DV.Debug("context.d.b: "..tostring(context.d.b))
    DV.Debug("context.d.c: "..tostring(context.d.c))

    context.a = 1
    DV.Debug("context.a: "..tostring(context.a))
    context.b = "New String"
    DV.Debug("context.b: "..tostring(context.b))
    context.c = context.d.c
    DV.Debug("context.c: "..tostring(context.c))
    context.d.a = 2    
    DV.Debug("context.d.a: "..tostring(context.d.a))
    context.d.b = "New WideString"
    DV.Debug("context.d.b: "..tostring(context.d.b))

    DV.Debug("context.a + context.d.a: "..(context.a + context.d.a))

    DV.Debug("context.v: "..tostring(context.v))
    DV.Debug("Length #context.v: "..tostring(#context.v))
    context.v[3] = 999;
    DV.Debug("----- index for -----")
    for i = 1, #context.v do
        DV.Debug("  context.v["..i.."]: "..context.v[i])
    end
end

)script";

static const String sss = R"script(

function main(arg1, arg2, arg3, arg4)
    DV.Debug(type(arg1).." > "..tostring(arg1))
    DV.Debug(type(arg2).." > "..tostring(arg2))
    DV.Debug(type(arg3).." > "..tostring(arg3))
    DV.Debug(type(arg4).." > "..tostring(arg4))
    return arg1, arg2, arg3, arg4
end

)script";

ScriptingTest::ScriptingTest(GameCore* g)
    : BaseScreen(g, "ScriptingTest")
{
}

void ScriptingTest::LoadResources()
{
    BaseScreen::LoadResources();

    DemoObj obj;
    obj.v.assign({ 1, 2, 3, 4, 5 });

    Reflection objRef = Reflection::Create(&obj).ref;

    try
    {
        LuaScript s;
        s.RunString(sss);
        Vector<Any> res = s.RunMain({ 1, "String", false, objRef });

        for (Any& val : res)
        {
            DAVA::Logger::Debug("Ret val: %s", val.GetType()->GetName());
        }
    }
    catch (const LuaException& e)
    {
        DAVA::Logger::Debug("LuaException: %s", e.what());
    }
}

void ScriptingTest::UnloadResources()
{
    BaseScreen::UnloadResources();
    //TODO: Release resources here
}

void ScriptingTest::Update(DAVA::float32 timeElapsed)
{
}
