/*
 * Copyright © 2017-2019 Intel Corporation
 * Simplified surfaceless platform for Nintendo Switch
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "egl_dri2.h"
#include "eglglobals.h"

// Simplified image allocation for Switch
static __DRIimage *
switch_alloc_image(struct dri2_egl_display *dri2_dpy,
                   struct dri2_egl_surface *dri2_surf)
{
   if (!dri2_dpy->image || !dri2_dpy->image->createImage)
      return NULL;
      
   return dri2_dpy->image->createImage(
      dri2_dpy->dri_screen_render_gpu, dri2_surf->base.Width,
      dri2_surf->base.Height, dri2_surf->visual, 0, NULL);
}

static void
switch_free_images(struct dri2_egl_surface *dri2_surf)
{
   struct dri2_egl_display *dri2_dpy =
      dri2_egl_display(dri2_surf->base.Resource.Display);

   if (dri2_surf->front && dri2_dpy->image && dri2_dpy->image->destroyImage) {
      dri2_dpy->image->destroyImage(dri2_surf->front);
      dri2_surf->front = NULL;
   }

   free(dri2_surf->swrast_device_buffer);
   dri2_surf->swrast_device_buffer = NULL;
}

static int
switch_image_get_buffers(__DRIdrawable *driDrawable, unsigned int format,
                         uint32_t *stamp, void *loaderPrivate,
                         uint32_t buffer_mask,
                         struct __DRIimageList *buffers)
{
   struct dri2_egl_surface *dri2_surf = loaderPrivate;
   struct dri2_egl_display *dri2_dpy =
      dri2_egl_display(dri2_surf->base.Resource.Display);

   buffers->image_mask = 0;
   buffers->front = NULL;
   buffers->back = NULL;

   if (buffer_mask & __DRI_IMAGE_BUFFER_FRONT) {
      if (!dri2_surf->front)
         dri2_surf->front = switch_alloc_image(dri2_dpy, dri2_surf);

      if (dri2_surf->front) {
         buffers->image_mask |= __DRI_IMAGE_BUFFER_FRONT;
         buffers->front = dri2_surf->front;
      }
   }

   return 1;
}

static _EGLSurface *
switch_create_surface(_EGLDisplay *disp, EGLint type,
                      _EGLConfig *conf, const EGLint *attrib_list)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_config *dri2_conf = dri2_egl_config(conf);
   struct dri2_egl_surface *dri2_surf;
   const __DRIconfig *config;

   dri2_surf = calloc(1, sizeof *dri2_surf);
   if (!dri2_surf)
      return NULL;

   if (!dri2_init_surface(&dri2_surf->base, disp, type, conf, attrib_list, false, NULL)) {
      free(dri2_surf);
      return NULL;
   }

   config = dri2_get_dri_config(dri2_conf, type, EGL_COLORSPACE_LINEAR);
   if (!config) {
      _eglError(EGL_BAD_MATCH, "Unsupported surfaceless configuration");
      goto cleanup_surface;
   }

   if (!dri2_create_drawable(dri2_dpy, config, dri2_surf, dri2_surf)) {
      _eglError(EGL_BAD_ALLOC, "Failed to create drawable");
      goto cleanup_surface;
   }

   return &dri2_surf->base;

cleanup_surface:
   free(dri2_surf);
   return NULL;
}

static EGLBoolean
switch_destroy_surface(_EGLDisplay *disp, _EGLSurface *surf)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   struct dri2_egl_surface *dri2_surf = dri2_egl_surface(surf);

   switch_free_images(dri2_surf);

   if (dri2_surf->dri_drawable)
      dri2_dpy->core->destroyDrawable(dri2_surf->dri_drawable);

   return EGL_TRUE;
}

static void
switch_flush_front_buffer(__DRIdrawable *driDrawable, void *loaderPrivate)
{
   // No-op for Switch - no display to flush to
}

static unsigned
switch_get_capability(void *loaderPrivate, enum dri_loader_cap cap)
{
   switch (cap) {
   case DRI_LOADER_CAP_RGBA_BIT:
      return 1;
   case DRI_LOADER_CAP_FP16:
      return 1;
   default:
      return 0;
   }
}

// Simplified device probing for Switch
static bool
switch_probe_device(_EGLDisplay *disp)
{
   struct dri2_egl_display *dri2_dpy = dri2_egl_display(disp);
   
   // For Switch, we always use software rendering
   dri2_dpy->fd_render_gpu = -1;
   dri2_dpy->fd_display_gpu = -1;
   
   return true;
}

// Switch-specific display virtual table
static const struct dri2_egl_display_vtbl switch_display_vtbl = {
   .create_surface = switch_create_surface,
   .destroy_surface = switch_destroy_surface,
   .create_image = dri2_create_image_khr,
   .destroy_image = dri2_destroy_image_khr,
   .swap_interval = dri2_swap_interval,
   .swap_buffers = dri2_swap_buffers,
   .swap_buffers_with_damage = dri2_swap_buffers_with_damage,
   .post_sub_buffer = dri2_post_sub_buffer,
   .copy_buffers = dri2_copy_buffers,
   .query_buffer_age = dri2_query_buffer_age,
   .create_wayland_buffer_from_image = dri2_create_wayland_buffer_from_image,
   .get_sync_values = dri2_get_sync_values_chromium,
   .get_msc_rate = dri2_get_msc_rate_angle,
   .image_get_buffers = switch_image_get_buffers,
   .flush_front_buffer = switch_flush_front_buffer,
   .get_capability = switch_get_capability,
};

// Initialize Switch surfaceless platform
EGLBoolean
switch_initialize_surfaceless(_EGLDisplay *disp)
{
   struct dri2_egl_display *dri2_dpy;

   dri2_dpy = calloc(1, sizeof *dri2_dpy);
   if (!dri2_dpy)
      return _eglError(EGL_BAD_ALLOC, "switch");

   disp->DriverData = (void *) dri2_dpy;
   dri2_dpy->vtbl = &switch_display_vtbl;

   if (!switch_probe_device(disp)) {
      _eglError(EGL_NOT_INITIALIZED, "switch: failed to probe device");
      goto cleanup_display;
   }

   if (!dri2_setup_extensions(disp)) {
      _eglError(EGL_NOT_INITIALIZED, "switch: failed to setup extensions");
      goto cleanup_display;
   }

   if (!dri2_create_screen(disp)) {
      _eglError(EGL_NOT_INITIALIZED, "switch: failed to create screen");
      goto cleanup_display;
   }

   if (!dri2_setup_device(disp, true)) {
      _eglError(EGL_NOT_INITIALIZED, "switch: failed to setup device");
      goto cleanup_display;
   }

   return EGL_TRUE;

cleanup_display:
   free(dri2_dpy);
   return EGL_FALSE;
} 