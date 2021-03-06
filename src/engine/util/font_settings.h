namespace Util {
    AssetID DEBUG_FONT;
    const f32 DEBUG_EDGE = 0.20;
    const Vec4 DEBUG_COLORS[] = {
        V4(1, 1, 1, 1),
        V4(0.75, 0.75, 0.5, 1),
        V4(0.5, 0.75, 0.5, 1),
    };

    b8 init() {
        DEBUG_FONT = Asset::fetch_id("MONACO_FONT");
        if (DEBUG_FONT != Asset::ASSET_ID_NO_ASSET)
            return true;
        LOG("Failed to load the default font.");
        return false;
    }

    // TODO(ed): Maybe don't calculate the font size this
    // many times per frame...
    f32 debug_font_size() {
        return 400 / (f32) Renderer::get_window_width();
    }

    f32 debug_line_height() {
        return Renderer::messure_text("A", debug_font_size(), DEBUG_FONT).y;
    }

    f32 debug_top_of_screen() {
        return Renderer::fetch_camera()->aspect_ratio + debug_line_height() / 2.0;
    }

    void debug_text(const char *text, f32 x, f32 y, u32 color_code = 0) {
        ASSERT(color_code < LEN(DEBUG_COLORS), "Invalid color code!");
        Renderer::draw_text(text, x, y, debug_font_size(), DEBUG_FONT,
                            0,
                            DEBUG_COLORS[color_code], DEBUG_EDGE, true);
    }

    void debug_text(const char *text, f32 y, u32 color_code = 0) {
        debug_text(text, -1, y, color_code);
    }
};
