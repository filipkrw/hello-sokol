#include "sokol_gfx.h"
#include "sokol_gp.h"
#include "sokol_app.h"
#include "sokol_glue.h"
#include "sokol_log.h"
#include <math.h>

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

static void frame(void)
{
    int width = sapp_width();
    int height = sapp_height();

    // Begin a new Sokol GP command queue (in pixel coordinates):
    sgp_begin(width, height);

    // Viewport covers the entire window:
    sgp_viewport(0, 0, width, height);

    // Orthographic projection from (0,0) to (width,height):
    //   left=0, right=width, top=0, bottom=height
    sgp_project(0.0f, (float)width, (float)height, 0.0f);

    // Clear the screen:
    sgp_set_color(0.08f, 0.08f, 0.08f, 1.0f);
    sgp_clear();

    // Set a fill color:
    sgp_set_color(0.6f, 0.6f, 0.6f, 1.0f);

    sgp_draw_filled_rounded_rect(
        24.f, 24.f,                                // x, y
        (float)width - 48.f, (float)height - 48.f, // w, h
        12.f,                                      // r
        48                                         // segments
    );

    // Prepare to render to the default swapchain:
    sg_pass pass = {.swapchain = sglue_swapchain()};
    sg_begin_pass(&pass);

    // Flush our queued Sokol GP commands:
    sgp_flush();
    sgp_end();

    // End pass and commit:
    sg_end_pass();
    sg_commit();
}

static void init(void)
{
    /* Setup Sokol GFX */
    sg_setup(&(sg_desc){
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });
    /* Setup Sokol GP */
    sgp_setup(&(sgp_desc){});
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