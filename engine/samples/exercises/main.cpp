#include <cubos/engine/cubos.hpp>
#include <cubos/engine/renderer/plugin.hpp>
#include <cubos/engine/renderer/point_light.hpp>
#include <cubos/engine/renderer/environment.hpp>
#include <cubos/engine/transform/plugin.hpp>
#include <cubos/engine/settings/settings.hpp>

using cubos::core::ecs::Commands;
using cubos::core::ecs::Write;

using namespace cubos::engine;

static void setPaletteSystem(Write<Renderer> renderer)
{
    (*renderer)->setPalette(VoxelPalette{{
        {{1, 0, 0, 1}},
        {{0, 1, 0, 1}},
        {{0, 0, 1, 1}},
    }});
}

static void spawnVoxelGridSystem(Commands commands, Write<Assets> assets)
{
    // Create a 2x2x2 voxel grid with a red cube on a corner
    auto gridAsset = assets->create(VoxelGrid{{2, 2, 2}, {1, 0, 0, 0, 0, 0, 0, 0}});

    commands.create(RenderableGrid{gridAsset, {-1.0f, 0.0f, -1.0f}}, LocalToWorld{});
}

static void spawnLightSystem(Commands commands)
{
    commands.create(
        PointLight{.color = {1.0f, 1.0f, 1.0f}, .intensity = 1.0f, .range = 10.0f},
        Position{{1.0f, 3.0f, -2.0f}}
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
        Position{{-3.0f, 1.0f, -3.0f}},
        Rotation{glm::quatLookAt(glm::normalize(glm::vec3{1.0f, 0.0f, 1.0f}),
            glm::vec3{0.0f, 1.0f, 0.0f})}
    ).entity();
}

static void settingsSystem(Write<Settings> settings)
{
    // We don't load assets in this sample and we don't even have an assets folder, so we should
    // disable assets IO.
    settings->setBool("assets.io.enabled", false);
}

int main()
{
    Cubos cubos{};

    cubos.addPlugin(rendererPlugin);

    cubos.startupSystem(settingsSystem).tagged("cubos.settings");

    cubos.startupSystem(setPaletteSystem).after("cubos.renderer.init");
    cubos.startupSystem(spawnVoxelGridSystem);
    cubos.startupSystem(spawnLightSystem);
    cubos.startupSystem(setEnvironmentSystem);
    cubos.startupSystem(spawnCameraSystem);

    cubos.run();
}
