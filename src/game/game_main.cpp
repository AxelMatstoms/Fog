// Tell the engine that this is loaded
#define FOG_GAME

#include <vector>

namespace Game {

Renderer::ParticleSystem system;

Physics::ShapeID square;

Physics::Body a, b;

struct A : public Logic::Entity {
    virtual void update(f32 delta){};
    virtual void draw() {
        Renderer::push_point(layer, position, V4(0, 0, 0, 1), 0.1);
    };

    REGISTER_NO_FIELDS(A_TYPE, A)
};

struct MyEnt : public Logic::Entity {
    void update(f32 delta) override {}

    void draw() override {
        Renderer::push_sprite(layer, position, scale, rotation,
                              ASSET_DEBUG_TEST,
                              LERP(V2(0, 0), value, V2(100, 100)), V2(64, 64));
    }

    f32 value;

    REGISTER_FIELDS(MY_ENT, MyEnt, position, rotation, scale, value)
};

void show_buffer(char *buffer, void *tmp) {
    std::vector<int> *vec = (std::vector<int> *) tmp;
    buffer += Util::format_inplace(buffer, "(%d) ", vec->size());
    for (int v : *vec) buffer += Util::format_inplace(buffer, "%d ", v);
}

void show_int(char *buffer, void *info) {
    Util::format_inplace(buffer, "%d", *((int *) info));
}

void entity_registration() {
    REGISTER_TYPE(std::vector<int>, show_buffer);

    REGISTER_ENTITY(A);
    REGISTER_ENTITY(MyEnt);
}

void setup() {
    using namespace Input;
    add(K(a), Name::LEFT);
    add(K(d), Name::RIGHT);
    add(K(w), Name::UP);
    add(K(s), Name::DOWN);

    Renderer::fetch_camera(0)->zoom = 2;

    system = Renderer::create_particle_system(5, 200, V2(0, 0));
    system.keep_alive = true;
    system.one_color = false;
    system.one_size = true;
    system.alive_time = {0.2, 0.4};
    system.velocity_dir = {0, 2*PI};
    system.spawn_size = {0.01, 0.03};
    system.die_red = {0.96, 0.96};
    system.die_green = {0.894, 0.894};
    system.die_blue = {0.529, 0.529};
    system.velocity = {0.1, 0.5};
}

// Main logic
void update(f32 delta) {
    system.update(delta);

    using namespace Input;
    if (down(Name::LEFT))
        system.spawn();
}

// Main draw
void draw() {
    system.draw();
}

}  // namespace Game
