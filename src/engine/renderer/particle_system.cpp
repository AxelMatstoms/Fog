namespace Renderer {

bool Particle::dead() {
    return (!keep_alive) && alive > 1.0;
}

void Particle::update(f32 delta) {
    if (dead()) return;
    alive += inv_alive_time * delta;
    progress = MOD(alive, 1.0);  //TODO(gu)
    velocity += acceleration * delta;
    position += velocity * delta;
    velocity *= pow(damping, delta);
    rotation += angular_velocity * delta;
}

void Particle::render(u32 layer, Vec2 origin, s32 slot, Vec2 uv_min, Vec2 uv_dim) {
    if (dead()) return;
    Renderer::push_sprite(
        layer,
        slot,
        position + origin,
        dim * size(progress),
        rotation,
        uv_min,
        uv_dim,
        color(progress));
}

//TODO(gu) replace oldest particle when particle system is full ?
Particle ParticleSystem::generate() {
    ASSERT(particles, "Trying to use uninitalized/destroyed particle system");

    f32 first_size = spawn_size.random();
    f32 first_size_deriv = spawn_size_deriv.random();
    f32 second_size = one_size ? first_size : die_size.random();
    f32 second_size_deriv = spawn_size_deriv.random();

    Vec4 first_color = V4(spawn_red.random(), spawn_green.random(),
            spawn_blue.random(), spawn_alpha.random());
    f32 first_color_deriv = spawn_color_deriv.random();

    Vec4 second_color;
    f32 second_color_deriv;
    if (one_color) {
        second_color = first_color;
        second_color_deriv = first_color_deriv;
    } else {
        second_color = V4(die_red.random(), die_green.random(),
                die_blue.random(), first_color.w);
        second_color_deriv = die_color_deriv.random();
    }

    if (!one_alpha) {
        second_color.w = die_alpha.random();
    }
    return {
        0, 0,
            1.0f / alive_time.random(),
            keep_alive,

            rotation.random(),
            angular_velocity.random(),

            position + V2(position_x.random(), position_y.random()),
            rotate(V2(1, 0), velocity_dir.random()) * velocity.random(),
            rotate(V2(1, 0), acceleration_dir.random()) * acceleration.random(),
            damping.random(),

            get_std_progress_f32_func(first_size, second_size, first_size_deriv, second_size_deriv),

            V2(width.random(), height.random()),

            get_std_progress_vec4_func(first_color, second_color, first_color_deriv, second_color_deriv),
            (s16) (num_sub_sprites ?  random_int() % num_sub_sprites : -1),
    };
}

void ParticleSystem::spawn(u32 num_particles) {
    ASSERT(particles, "Trying to use uninitalized/destroyed particle system");
    for (u32 i = 0; i < num_particles; i++) {
        if (head == tail) return;
        Particle new_particle = generate();
        particles[tail] = new_particle;
        tail = (tail + 1) % max_num_particles;
    }
}

void ParticleSystem::update(f32 delta) {
    ASSERT(particles, "Trying to use uninitalized/destroyed particle system");
    u32 i = head;
    bool move = true;
    do {
        particles[i].update(delta);
        if (move && particles[i].dead()) {
            u32 new_head = (head + 1) % max_num_particles;
            if (new_head != tail) {
                head = new_head;
            }
        } else {
            move = false;
        }
    } while ((i = (i + 1) % max_num_particles) != tail);
}

void ParticleSystem::draw() {
    ASSERT(particles, "Trying to use uninitalized/destroyed particle system");
    u32 i = head;
    Vec2 p = relative ? position : V2(0, 0);
    do {
        i %= max_num_particles;
        if (num_sub_sprites) {
            SubSprite sprite = sub_sprites[particles[i].sprite];
            particles[i].render(layer, p, sprite.texture, sprite.min, sprite.dim);
        } else {
            particles[i].render(layer, p, -1, V2(0, 0), V2(0, 0));
        }
    } while ((i = (i + 1) % max_num_particles) != tail);
}

void ParticleSystem::add_sprite(AssetID texture, u32 u, u32 v, u32 w, u32 h){
    ASSERT(particles, "Trying to use uninitalized/destroyed particle system");
    SubSprite sub_sprite = {Asset::fetch_image(texture)->id,
        V2(u, v),
        V2(w, h)};
    ASSERT(num_sub_sprites != MAX_NUM_SUB_SPRITES,
            "Too manu subsprites in particle system");
    sub_sprites[num_sub_sprites++] = sub_sprite;
}

ParticleSystem create_particle_system(u32 layer, u32 num_particles, Vec2 position) {
    ASSERT(num_particles > 1, "Too small particle system");
    Util::MemoryArena *arena = Util::request_arena();
    Particle *particles = arena->push<Particle>(num_particles);
    for (u32 i = 0; i < num_particles; i++) {
        particles[i].alive = 2.0;
    }
    ParticleSystem particle_system = {arena, 0, 1};
    particle_system.head = 1;
    particle_system.max_num_particles = num_particles;
    particle_system.particles = particles;
    particle_system.layer = layer;

    particle_system.relative = false;
    particle_system.keep_alive = false;
    particle_system.one_color = true;
    particle_system.one_alpha = false;
    particle_system.one_size = false;

    particle_system.num_sub_sprites = 0;

    particle_system.position = position;

    particle_system.alive_time = {2, 2};
    particle_system.rotation = {0, 2 * PI};
    particle_system.angular_velocity = {0, 0};

    particle_system.spawn_size = {0.5, 1.0};
    particle_system.die_size = {0.0, 0.0};

    particle_system.width = {1.0, 1.0};
    particle_system.height = {1.0, 1.0};

    particle_system.position_x = {0.0, 0.0};
    particle_system.position_y = {0.0, 0.0};
    particle_system.velocity_dir = {PI / 2.0, PI / 2.0};
    particle_system.velocity = {3.0, 5.0};
    particle_system.damping = {0.90, 1.0};
    particle_system.acceleration_dir = {-PI / 2.0, -PI / 2.0};
    particle_system.acceleration = {0.0, 0.0};

    particle_system.spawn_red = {1.0, 1.0};
    particle_system.spawn_green = {1.0, 1.0};
    particle_system.spawn_blue = {1.0, 1.0};
    particle_system.spawn_alpha = {1.0, 1.0};

    particle_system.die_red = {};
    particle_system.die_green = {};
    particle_system.die_blue = {};
    particle_system.die_alpha = {};
    return particle_system;
}

void destroy_particle_system(ParticleSystem *system) {
    system->memory->pop();
    system->particles = nullptr;
}

};
