/*
 * Copyright © 2017-2019 Intel Corporation
 * Simplified EGL driver for Nintendo Switch - Software rendering only
 */

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <c11/threads.h>
#include <GL/gl.h>
#include <GL/internal/dri_interface.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "egl_dri2.h"
#include "egldefines.h"

// Direct softpipe driver linkage for Switch
extern const __DRIcoreExtension driCoreExtension;
extern const __DRImesaCoreExtension driMesaExtension;
extern const __DRIswrastExtension driSWRASTExtension;
extern const __DRIimageDriverExtension driImageDriverExtension;
extern const __DRIconfigOptionsExtension driConfigOptionsExtension;

// Forward declaration for Switch platform
EGLBoolean switch_initialize_surfaceless(_EGLDisplay *disp);

// Switch-specific EGL display structure
struct switch_egl_display {
   struct dri2_egl_display base;
   // Add any Switch-specific fields here
};

static struct switch_egl_display *
switch_egl_display(_EGLDisplay *disp)
{
   return (struct switch_egl_display *) disp;
}

static EGLBoolean
switch_initialize(_EGLDisplay *disp)
{
   // Use the simplified Switch surfaceless platform
   return switch_initialize_surfaceless(disp);
}

static void
switch_display_destroy(_EGLDisplay *disp)
{
   struct switch_egl_display *switch_dpy = switch_egl_display(disp);
   
   // Clean up the base DRI2 display
   dri2_display_destroy(disp);
   
   // Free Switch-specific resources
   free(switch_dpy);
}

// EGL driver interface
const struct _egl_driver _eglDriver = {
   .name = "switch",
   .type = _EGL_DRIVER_SOFTWARE,
   .api_mask = _EGL_API_OPENGL | _EGL_API_OPENGL_ES,
   .major_version = 1,
   .minor_version = 0,
   .initialize = switch_initialize,
   .terminate = dri2_terminate,
   .create_context = dri2_create_context,
   .destroy_context = dri2_destroy_context,
   .create_surface = dri2_create_surface,
   .destroy_surface = dri2_destroy_surface,
   .make_current = dri2_make_current,
   .swap_buffers = dri2_swap_buffers,
   .swap_buffers_with_damage = dri2_swap_buffers_with_damage,
   .swap_interval = dri2_swap_interval,
   .bind_tex_image = dri2_bind_tex_image,
   .release_tex_image = dri2_release_tex_image,
   .query_surface = dri2_query_surface,
   .display_destroy = switch_display_destroy,
}; 