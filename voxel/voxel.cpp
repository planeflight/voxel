#include "omega/core/app.hpp"
#include "imgui/imgui.h"
#include "omega/events/event.hpp"
#include "omega/scene/imgui.hpp"

using namespace omega;

struct VoxelGame : public core::App {
    VoxelGame(const core::AppConfig &config) : core::App::App(config) {

    }
    
    void setup() override {

    }
    
    void render(f32 dt) override {
    }

    void update(f32 dt) override {

    }
    
    void input(f32 dt) override {
        auto &keys = globals->input.get_key_manager();
        if (keys.key_pressed(events::Key::k_escape)) {
            running = false;
        }
    }
};

int main() {
    core::AppConfig config;
    config.resizable = true;
    config.width = 1920;
    config.height = 1080;
    config.imgui = true;
    config.title = "GAME";

    VoxelGame app{config};
    app.run();
}

