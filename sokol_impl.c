#define SOKOL_APP_IMPL
#define SOKOL_D3D11
#define SOKOL_GFX_IMPL
#define SOKOL_GLUE_IMPL
#define SOKOL_LOG_IMPL

#define SOKOL_WIN32_FORCE_MAIN
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#define SOKOL_GP_IMPL
#include "sokol_gp.h"

#define SOKOL_GL_IMPL
#include "util/sokol_gl.h"

#define FONTSTASH_IMPLEMENTATION
#include "lib/fontstash.h"

#define SOKOL_FONTSTASH_IMPL
#include "util/sokol_fontstash.h"
