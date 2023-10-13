#include <cubos/engine/renderer/plugin.hpp>
#include <cubos/engine/renderer/point_light.hpp>
#include <cubos/engine/renderer/environment.hpp>
#include <cubos/engine/transform/plugin.hpp>
#include <cubos/engine/settings/settings.hpp>
#include <cubos/engine/voxels/plugin.hpp>
#include <cubos/engine/input/plugin.hpp>

#include "components.hpp"

using cubos::core::ecs::Commands;
using cubos::core::ecs::Read;
using cubos::core::ecs::Write;
using cubos::core::ecs::Query;

using namespace cubos::engine;

static const Asset<VoxelGrid> CastleAsset = AnyAsset("fcb15958-8fb4-4d2b-8c8b-607b231729cf");
static const Asset<VoxelPalette> PaletteAsset = AnyAsset("3e3b90da-59a2-4cea-8cf9-bb2904abb7e9");
static const Asset<InputBindings> BindingsAsset = AnyAsset("bf49ba61-5103-41bc-92e0-8a442d7842c3");

static void setPaletteSystem(Write<Renderer> renderer, Read<Assets> assets)
{
    auto palette = assets->read(PaletteAsset);
    (*renderer)->setPalette(*palette);
}

static void spawnCastleSystem(Commands commands, Read<Assets> assets)
{
    // Load castle model
    auto castle = assets->read(CastleAsset);
    // Calculate the necessary offset to center the model on (0, 0, 0).
    glm::vec3 offset = glm::vec3(castle->size().x, castle->size().y, castle->size().z) / -2.0F;

    commands.create(RenderableGrid{CastleAsset, offset}, LocalToWorld{}, Rotation{}, InputRotation{});
}

static void spawnLightSystem(Commands commands)
{
    commands.create(
        PointLight{.color = {1.0f, 1.0f, 1.0f}, .intensity = 4.0f, .range = 100.0f},
        Position{{10.0f, 30.0f, -20.0f}}
    );
}

static void setEnvironmentSystem(Write<RendererEnvironment> env)
{
    env->ambient = {0.2f, 0.2f, 0.2f};
    env->skyGradient[0] = {0.6f, 0.6f, 0.4f};
    env->skyGradient[1] = {0.6f, 0.6f, 0.8f};
}

static void spawnCameraSystem(Commands commands, Write<ActiveCameras> cameras)
{
    cameras->entities[0] = commands.create(
        Camera{.fovY = 60.0f, .zNear = 0.1f, .zFar = 100.0f},
        Position{{0.0f, 10.0f, -45.0f}},
        Rotation{glm::quatLookAt(glm::normalize(glm::vec3{0.0f, -10.0f, 45.0f}),
            glm::vec3{0.0f, 1.0f, 0.0f})}
    ).entity();
}

static void initInputSystem(Read<Assets> assets, Write<Input> input)
{
    auto bindings = assets->read<InputBindings>(BindingsAsset);
    input->bind(*bindings);
}

static void settingsSystem(Write<Settings> settings)
{
    settings->setString("assets.io.path", SAMPLE_ASSETS_FOLDER);
}

static void castleRotateSystem(Query<Write<Rotation>, Read<InputRotation>> query, Read<DeltaTime> dt, 
    Read<Input> input)
{
    if (input->axis("horizontal") != 0.0F || input->axis("vertical") != 0.0F)
    {
        auto rot = glm::quat(glm::vec3{input->axis("vertical")*dt->value, input->axis("horizontal")*dt->value, 0.0f});
        for (auto [entity, rotation, auto_rotation] : query)
        {
            rotation->quat = glm::cross(rot, rotation->quat);
        }
    }
}

int main()
{
    Cubos cubos{};

    cubos.addPlugin(rendererPlugin);
    cubos.addPlugin(voxelsPlugin);
    cubos.addPlugin(inputPlugin);

    cubos.addComponent<InputRotation>();

    cubos.startupSystem(settingsSystem).tagged("cubos.settings");

    cubos.startupSystem(setPaletteSystem).after("cubos.renderer.init");
    cubos.startupSystem(spawnLightSystem);
    cubos.startupSystem(setEnvironmentSystem);
    cubos.startupSystem(spawnCameraSystem);
    cubos.startupSystem(spawnCastleSystem).tagged("cubos.assets");
    cubos.startupSystem(initInputSystem).tagged("cubos.assets");

    cubos.system(castleRotateSystem);

    cubos.run();
}
