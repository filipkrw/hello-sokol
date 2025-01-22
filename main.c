#include "sokol_gfx.h"
#include "sokol_gp.h"
#include "sokol_app.h"
#include "sokol_glue.h"
#include "sokol_log.h"
#include "util/sokol_gl.h"
#include "lib/fontstash.h"
#include "util/sokol_fontstash.h"
#include <math.h>

static FONScontext *fs;
static int fontNormal;

static void draw_corner_arc(float cx, float cy, float radius,
                            float start_angle, float end_angle,
                            int segments)
{
    if (segments < 1)
    {
        segments = 1;
    }
    /* We'll store up to segments+2 points in a small local array.
       For large segments, you might want dynamic allocation. */
    if (segments > 63)
    {
        segments = 63;
    }

    sgp_point pts[64];
    pts[0].x = cx;
    pts[0].y = cy;

    float step = (end_angle - start_angle) / (float)segments;
    for (int i = 0; i <= segments; i++)
    {
        float a = start_angle + i * step;
        pts[i + 1].x = cx + cosf(a) * radius;
        pts[i + 1].y = cy + sinf(a) * radius;
    }

    /* Now draw them as a triangle strip. This produces a filled arc. */
    // sgp_draw_filled_triangles_strip(pts, (uint32_t)(segments + 2));
    // Pseudocode
    for (int i = 0; i < segments + 1; i++)
    {
        sgp_draw_filled_triangle(
            cx, cy,                    // center
            pts[i].x, pts[i].y,        // arc[i]
            pts[i + 1].x, pts[i + 1].y // arc[i+1]
        );
    }
}

static void sgp_draw_filled_rounded_rect(float x, float y,
                                         float w, float h,
                                         float r, /* corner radius */
                                         int segments)
{
    /* 1) The middle rectangle (avoiding corners). */
    sgp_draw_filled_rect(x + r, y + r, w - 2 * r, h - 2 * r);

    /* 2) Bars along the top/bottom edges (minus the corners). */
    sgp_draw_filled_rect(x + r, y, w - 2 * r, r);         // top bar
    sgp_draw_filled_rect(x + r, y + h - r, w - 2 * r, r); // bottom bar

    /* 3) Bars along the left/right edges (minus the corners). */
    sgp_draw_filled_rect(x, y + r, r, h - 2 * r);         // left bar
    sgp_draw_filled_rect(x + w - r, y + r, r, h - 2 * r); // right bar

    /* 4) Each corner arc, approximated with a triangle fan of `segments` steps. */

    /* top-left corner: angles go from π to 1.5π */
    draw_corner_arc(x + r, y + r, r, (float)M_PI, 1.5f * (float)M_PI, segments);

    /* top-right corner: angles from 1.5π to 2π
       or equivalently -0.5π to 0.0, whichever you prefer */
    draw_corner_arc(x + w - r, y + r, r, 1.5f * (float)M_PI, 2.0f * (float)M_PI, segments);

    /* bottom-right corner: 0 to 0.5π */
    draw_corner_arc(x + w - r, y + h - r, r, 0.0f, 0.5f * (float)M_PI, segments);

    /* bottom-left corner: 0.5π to π */
    draw_corner_arc(x + r, y + h - r, r, 0.5f * (float)M_PI, (float)M_PI, segments);
}

void draw_rounded_rect(float x, float y, float w, float h, float r, int segments)
{
    // 1) Center
    sgl_begin_quads();
    sgl_v2f(x + r, y + r);
    sgl_v2f(x + w - r, y + r);
    sgl_v2f(x + w - r, y + h - r);
    sgl_v2f(x + r, y + h - r);
    sgl_end();

    // 2) Edges
    // Top
    sgl_begin_quads();
    sgl_v2f(x + r, y);
    sgl_v2f(x + w - r, y);
    sgl_v2f(x + w - r, y + r);
    sgl_v2f(x + r, y + r);
    sgl_end();
    // Bottom
    sgl_begin_quads();
    sgl_v2f(x + r, y + h - r);
    sgl_v2f(x + w - r, y + h - r);
    sgl_v2f(x + w - r, y + h);
    sgl_v2f(x + r, y + h);
    sgl_end();
    // Left
    sgl_begin_quads();
    sgl_v2f(x, y + r);
    sgl_v2f(x + r, y + r);
    sgl_v2f(x + r, y + h - r);
    sgl_v2f(x, y + h - r);
    sgl_end();
    // Right
    sgl_begin_quads();
    sgl_v2f(x + w - r, y + r);
    sgl_v2f(x + w, y + r);
    sgl_v2f(x + w, y + h - r);
    sgl_v2f(x + w - r, y + h - r);
    sgl_end();

    // 3) Corners (triangle fans)
    // Top-left
    {
        float cx = x + r, cy = y + r;
        float step = (M_PI * 0.5f) / segments;
        for (int i = 0; i < segments; i++)
        {
            float a0 = M_PI + i * step;
            float a1 = M_PI + (i + 1) * step;
            sgl_begin_triangles();
            sgl_v2f(cx, cy);
            sgl_v2f(cx + cosf(a0) * r, cy + sinf(a0) * r);
            sgl_v2f(cx + cosf(a1) * r, cy + sinf(a1) * r);
            sgl_end();
        }
    }
    // Top-right
    {
        float cx = x + w - r, cy = y + r;
        float step = (M_PI * 0.5f) / segments;
        for (int i = 0; i < segments; i++)
        {
            float a0 = 1.5f * M_PI + i * step;
            float a1 = 1.5f * M_PI + (i + 1) * step;
            sgl_begin_triangles();
            sgl_v2f(cx, cy);
            sgl_v2f(cx + cosf(a0) * r, cy + sinf(a0) * r);
            sgl_v2f(cx + cosf(a1) * r, cy + sinf(a1) * r);
            sgl_end();
        }
    }
    // Bottom-right
    {
        float cx = x + w - r, cy = y + h - r;
        float step = (M_PI * 0.5f) / segments;
        for (int i = 0; i < segments; i++)
        {
            float a0 = 0.0f + i * step;
            float a1 = 0.0f + (i + 1) * step;
            sgl_begin_triangles();
            sgl_v2f(cx, cy);
            sgl_v2f(cx + cosf(a0) * r, cy + sinf(a0) * r);
            sgl_v2f(cx + cosf(a1) * r, cy + sinf(a1) * r);
            sgl_end();
        }
    }
    // Bottom-left
    {
        float cx = x + r, cy = y + h - r;
        float step = (M_PI * 0.5f) / segments;
        for (int i = 0; i < segments; i++)
        {
            float a0 = 0.5f * M_PI + i * step;
            float a1 = 0.5f * M_PI + (i + 1) * step;
            sgl_begin_triangles();
            sgl_v2f(cx, cy);
            sgl_v2f(cx + cosf(a0) * r, cy + sinf(a0) * r);
            sgl_v2f(cx + cosf(a1) * r, cy + sinf(a1) * r);
            sgl_end();
        }
    }
}

static void frame(void)
{
    // Begin the default pass
    sg_pass pass = {.swapchain = sglue_swapchain()};
    sg_begin_pass(&pass);

    // Fetch current window size
    int width = sapp_width();
    int height = sapp_height();

    // Set up an orthographic projection so we can use screen coordinates
    sgl_defaults();
    sgl_matrix_mode_projection();
    sgl_load_identity();
    sgl_ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, +1.0f);

    // Optionally switch to modelview (identity), if needed
    sgl_matrix_mode_modelview();
    sgl_load_identity();

    sgl_begin_quads();
    sgl_c4f(0.0f, 0.0f, 0.0f, 1.0f);
    sgl_v2f(0.0f, 0.0f);
    sgl_v2f((float)width, 0.0f);
    sgl_v2f((float)width, (float)height);
    sgl_v2f(0.0f, (float)height);
    sgl_end();

    sgl_c4f(1.0f, 1.0f, 1.0f, 1.0f);
    draw_rounded_rect(100.0f, 100.f, 100.0f, 100.0f, 20.0f, 16);

    fonsClearState(fs);
    fonsSetSize(fs, 24.0f * 8);
    fonsSetFont(fs, fontNormal);
    fonsSetColor(fs, 0xffffffff); /* White in RGBA */
    fonsDrawText(fs, 260.0f, 208.0f, "Hej Zu!", NULL);

    /* Flush FontStash so it actually renders */
    sfons_flush(fs);

    // Important: draw the batched geometry
    sgl_draw();

    sg_end_pass();
    sg_commit();
}

static void init(void)
{
    sg_setup(&(sg_desc){
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });
    sgl_setup(&(sgl_desc_t){
        .logger.func = slog_func,
    });
    // sgp_setup(&(sgp_desc){});

    fs = sfons_create(&(sfons_desc_t){
        .width = 2048,
        .height = 2048,
    });

    fontNormal = fonsAddFont(fs, "sans", "DroidSerif-Regular.ttf");
    fonsSetFont(fs, fontNormal);
}
static void cleanup(void)
{
    sgp_shutdown();
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char *argv[])
{

    (void)argc;
    (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .width = 800,
        .height = 600,
        .sample_count = 4,
        .window_title = "Rounded Rect (Sokol GP)",
        .logger.func = slog_func,
        .high_dpi = true,
    };
}