/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/**
 * SECTION:shell-glsl-quad
 * @short_description: Draw a rectangle using GLSL
 *
 * A #ShellGLSLQuad draws one single rectangle, sized to the allocation
 * box, but allows running custom GLSL to the vertex and fragment
 * stages of the graphic pipeline.
 *
 * To ease writing the shader, a single texture layer is also used.
 */

#include "config.h"

#include <cogl/cogl.h>
#include "shell-glsl-quad.h"

typedef struct _ShellGLSLQuadPrivate ShellGLSLQuadPrivate;
struct _ShellGLSLQuadPrivate
{
  CoglPipeline  *pipeline;
};

G_DEFINE_TYPE_WITH_PRIVATE (ShellGLSLQuad, shell_glsl_quad, CLUTTER_TYPE_ACTOR);

static gboolean
shell_glsl_quad_get_paint_volume (ClutterActor       *actor,
                                  ClutterPaintVolume *volume)
{
  return clutter_paint_volume_set_from_allocation (volume, actor);
}

static void
shell_glsl_quad_paint (ClutterActor *actor)
{
  ShellGLSLQuad *self = SHELL_GLSL_QUAD (actor);
  ShellGLSLQuadPrivate *priv;
  guint8 paint_opacity;
  ClutterActorBox box;

  priv = shell_glsl_quad_get_instance_private (self);

  paint_opacity = clutter_actor_get_paint_opacity (actor);
  clutter_actor_get_allocation_box (actor, &box);

  cogl_pipeline_set_color4ub (priv->pipeline,
                              paint_opacity,
                              paint_opacity,
                              paint_opacity,
                              paint_opacity);
  cogl_framebuffer_draw_rectangle (cogl_get_draw_framebuffer (),
                                   priv->pipeline,
                                   box.x1, box.y1,
                                   box.x2, box.y2);
}


/**
 * shell_glsl_quad_add_glsl_snippet:
 * @quad: a #ShellGLSLQuad
 * @hook: where to insert the code
 * @declarations: GLSL declarations
 * @code: GLSL code
 * @is_replace: wheter Cogl code should be replaced by the custom shader
 *
 * Adds a GLSL snippet to the pipeline used for drawing the actor texture.
 * See #CoglSnippet for details.
 *
 * This is only valid inside the a call to the build_pipeline() virtual
 * function.
 */
void
shell_glsl_quad_add_glsl_snippet (ShellGLSLQuad    *quad,
                                  ShellSnippetHook  hook,
                                  const char       *declarations,
                                  const char       *code,
                                  gboolean          is_replace)
{
  ShellGLSLQuadClass *klass = SHELL_GLSL_QUAD_GET_CLASS (quad);
  CoglSnippet *snippet;

  g_return_if_fail (klass->base_pipeline != NULL);

  if (is_replace)
    {
      snippet = cogl_snippet_new ((CoglSnippetHook)hook, declarations, NULL);
      cogl_snippet_set_replace (snippet, code);
    }
  else
    {
      snippet = cogl_snippet_new ((CoglSnippetHook)hook, declarations, code);
    }

  if (hook == SHELL_SNIPPET_HOOK_VERTEX ||
      hook == SHELL_SNIPPET_HOOK_FRAGMENT)
    cogl_pipeline_add_snippet (klass->base_pipeline, snippet);
  else
    cogl_pipeline_add_layer_snippet (klass->base_pipeline, 0, snippet);

  cogl_object_unref (snippet);
}

static void
shell_glsl_quad_dispose (GObject *gobject)
{
  ShellGLSLQuad *self = SHELL_GLSL_QUAD (gobject);
  ShellGLSLQuadPrivate *priv;

  priv = shell_glsl_quad_get_instance_private (self);

  g_clear_pointer (&priv->pipeline, cogl_object_unref);

  G_OBJECT_CLASS (shell_glsl_quad_parent_class)->dispose (gobject);
}

static void
shell_glsl_quad_init (ShellGLSLQuad *quad)
{
}

static void
shell_glsl_quad_constructed (GObject *object)
{
  ShellGLSLQuad *self;
  ShellGLSLQuadClass *klass;
  ShellGLSLQuadPrivate *priv;
  CoglContext *ctx =
    clutter_backend_get_cogl_context (clutter_get_default_backend ());

  G_OBJECT_CLASS (shell_glsl_quad_parent_class)->constructed (object);

  /* Note that, differently from ClutterBlurEffect, we are calling
     this inside constructed, not init, so klass points to the most-derived
     GTypeClass, not ShellGLSLQuadClass.
  */
  klass = SHELL_GLSL_QUAD_GET_CLASS (object);
  self = SHELL_GLSL_QUAD (object);
  priv = shell_glsl_quad_get_instance_private (self);

  if (G_UNLIKELY (klass->base_pipeline == NULL))
    {
      klass->base_pipeline = cogl_pipeline_new (ctx);
      cogl_pipeline_set_blend (klass->base_pipeline, "RGBA = ADD (SRC_COLOR * (SRC_COLOR[A]), DST_COLOR * (1-SRC_COLOR[A]))", NULL);

      if (klass->build_pipeline != NULL)
        klass->build_pipeline (self);
    }

  priv->pipeline = cogl_pipeline_copy (klass->base_pipeline);

  cogl_pipeline_set_layer_null_texture (priv->pipeline, 0);
}

static void
shell_glsl_quad_class_init (ShellGLSLQuadClass *klass)
{
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->constructed = shell_glsl_quad_constructed;
  gobject_class->dispose = shell_glsl_quad_dispose;

  actor_class->get_paint_volume = shell_glsl_quad_get_paint_volume;
  actor_class->paint = shell_glsl_quad_paint;
}

/**
 * shell_glsl_quad_get_uniform_location:
 * @quad: a #ShellGLSLQuad
 * @name: the uniform name
 *
 * Returns: the location of the uniform named @name, that can be
 *          passed to shell_glsl_quad_set_uniform_float().
 */
int
shell_glsl_quad_get_uniform_location (ShellGLSLQuad *quad,
                                      const char    *name)
{
  ShellGLSLQuadPrivate *priv = shell_glsl_quad_get_instance_private (quad);
  return cogl_pipeline_get_uniform_location (priv->pipeline, name);
}

/**
 * shell_glsl_quad_set_uniform_float:
 * @quad: a #ShellGLSLQuad
 * @uniform: the uniform location (as returned by shell_glsl_quad_get_uniform_location())
 * @n_components: the number of components in the uniform (eg. 3 for a vec3)
 * @total_count: the total number of floats in @value
 * @value: (array length=total_count): the array of floats to set @uniform
 */
void
shell_glsl_quad_set_uniform_float (ShellGLSLQuad *quad,
                                   int            uniform,
                                   int            n_components,
                                   int            total_count,
                                   const float   *value)
{
  ShellGLSLQuadPrivate *priv = shell_glsl_quad_get_instance_private (quad);
  cogl_pipeline_set_uniform_float (priv->pipeline, uniform,
                                   n_components, total_count / n_components,
                                   value);
}
