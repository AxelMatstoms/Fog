
namespace Util {

//
// Memory functions
//
static const u32 FRAME_LAG_FOR_MEMORY = 2;
static u32 CURRENT_MEMORY = 0;
static MemoryArena *FRAME_MEMORY[FRAME_LAG_FOR_MEMORY];

void do_all_allocations() {
    static_assert(TOTAL_MEMORY_BUDGET % ARENA_SIZE_IN_BYTES == 0);

    // Setup regions.
    global_memory.free_regions = global_memory.all_regions + 0;
    global_memory.num_free_regions = NUM_ARENAS;
    for (u64 i = 0; i < NUM_ARENAS - 1; i++) {
        global_memory.all_regions[i].next = global_memory.all_regions + i + 1;
        global_memory.all_regions[i].memory = malloc(ARENA_SIZE_IN_BYTES);
    }
    global_memory.all_regions[NUM_ARENAS - 1].next = 0;

    // Frame memory
    for (u32 i = 0; i < FRAME_LAG_FOR_MEMORY; i++)
        FRAME_MEMORY[i] = request_arena();
}

void swap_frame_memory() {
    CURRENT_MEMORY = (CURRENT_MEMORY + 1) % FRAME_LAG_FOR_MEMORY;
    // Clear when it is swapped to so the old ones
    // still can be used.
    FRAME_MEMORY[CURRENT_MEMORY]->clear();
}

template <typename T>
T *request_temporary_memory(u64 num) {
    return FRAME_MEMORY[CURRENT_MEMORY]->push<T>(num);
}

template <typename T>
T *temporary_push(T t) {
    return FRAME_MEMORY[CURRENT_MEMORY]->push(t);
}

MemoryArena *request_arena(bool only_one) {
    ASSERT(global_memory.free_regions, "No more memory");
    ASSERT(global_memory.num_free_regions, "No more memory");
    MemoryArena *next = global_memory.free_regions;
    global_memory.free_regions = next->next;
    --global_memory.num_free_regions;
    next->only_one = only_one;
    next->next = 0;
    next->watermark = 0;
    return next;
}

void return_arean(MemoryArena *arena) {
    ASSERT(arena, "nullptr is not a valid argument.");
    if (arena->next) return_arean(arena->next);
    ++global_memory.num_free_regions;
    arena->next = global_memory.free_regions;
    global_memory.free_regions = arena;
}

template <typename T>
T *MemoryArena::push(u64 count) {
    u64 allocation_size = sizeof(T) * count;
    ASSERT(allocation_size <= ARENA_SIZE_IN_BYTES, "Too large allocation");
    // TODO(ed): Should do boundry checking here.
    if (watermark + allocation_size > ARENA_SIZE_IN_BYTES) {
        if (!next) {
            if (only_one) HALT_AND_CATCH_FIRE;
            next = request_arena();
        }
        return next->push<T>(count);
    }
    void *region = (void *) (((u8 *) memory) + watermark);
    watermark += allocation_size;
    return (T *) region;
}

template <typename T>
T *MemoryArena::push(T type) {
    T* copy = push<T>(1);
    copy_bytes(&type, copy, sizeof(T));
    return copy;
}

void MemoryArena::clear() {
    // TODO(ed): Should do boundry checking here.
    while (next) {
        MemoryArena *old = next;
        next = next->next;
        return_arean(old);
    }
    watermark = 0;
}

void MemoryArena::pop() { return_arean(this); }

// TODO(ed):
// I know these are just wrappers for malloc
// and free, but they add an easy place to see
// if memory is potentially leaked, and are intended
// for use in the engine. Preferably they would be
// replaced some time later.

template <typename T>
T *push_memory(u32 num) {
    return (T *) malloc(sizeof(T) * num);
}

template <typename T>
T *resize_memory(T *data, u32 num) {
    return (T *) realloc(data, sizeof(T) * num);
}

template <typename T>
void pop_memory(T *data) {
    free(data);
}

void zero_memory(void *to, u64 size) {
    u8 *ptr = (u8 *) to;
    // TODO(ed): memcpy might be like... A lot faster..
    for (u64 i = 0; i <= size; i++) 
        ptr[i] = 0;
}

template <typename T>
void quicksort(T *start, u64 length, bool (*cmp)(T *a, T *b)) {
    // NOTE(ed): Not sure how well this quicksort performs,
    // I stole it from wikipedia but it looks quite promising.
    //
    // It might be possible to squeeze out even more performance
    // from this bad boy though.
    constexpr auto partition = [](T *start, u64 length,
                                  bool (*cmp)(T *a, T *b)) -> u64 {
        T *pivot = start + (length / 2);
        u64 l = 0;
        u64 h = length - 1;
        for (;;) {
            while (cmp(start + l, pivot)) l++;
            while (cmp(pivot, start + h)) h--;
            if (l >= h) return h;

            T tmp = start[l];
            start[l] = start[h];
            start[h] = tmp;

            l++;
            h--;
        }
    };

    if (1 < length) {
        u64 mid = partition(start, length, cmp);
        quicksort(start, mid, cmp);
        quicksort(start + mid, length - mid, cmp);
    }
}

void copy_bytes(void *from, void *to, u64 size) {
    // TODO(ed): memcpy might be like... A lot faster..
    u8 *_to = (u8 *) to;
    u8 *_from = (u8 *) from;
    while(size--) *(_to++) = *(_from++);
}

}  // namespace Util
