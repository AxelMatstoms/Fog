// Tell the engine that this is loaded
#define FOG_GAME

namespace Game {

Renderer::ParticleSystem system;

void entity_registration() {}

void setup() {
    Input::add(K(s), Input::Name::SPAWN);
    Input::add(K(u), Input::Name::UPDATE);

    Renderer::turn_on_camera(0);
    Renderer::get_camera(0)->zoom = 0.1;

    system = Renderer::create_particle_system(0, 100, V2(0, 0));
    for (u32 i = 0; i < 50; i++) {
        system.spawn();
    }
}

// Main logic
void update(f32 delta) {
    static bool updated = false;
    if (!updated) {
        LOG("updating particle system once");
        updated = true;
        system.update(delta);
    }

    if (Input::pressed(Input::Name::SPAWN)) {
        for (u32 i = 0; i < 50; i++) {
            system.spawn();
        }
        system.update(delta);
    }

    if (Input::down(Input::Name::UPDATE)) {
        system.update(delta);
    }
}

// Main draw
void draw() {
    system.draw();
}

}  // namespace Game
