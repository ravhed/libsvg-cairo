/* libsvg-cairo - Render SVG documents using the cairo library
 *
 * Copyright © 2002 University of Southern California
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Carl D. Worth <cworth@isi.edu>
 */

#include <stdlib.h>
#define __USE_XOPEN_EXTENDED
#include <string.h>

#include "svg-cairo-internal.h"
#if defined(_MSC_VER)
/* include M_PI from math.h */
#define _USE_MATH_DEFINES 1
#endif
#include "math.h"

static svg_status_t
_svg_cairo_begin_group (void *closure, double opacity, const char *id, const char *klass);

static svg_status_t
_svg_cairo_begin_element (void *closure, const char *id, const char *klass);

static svg_status_t
_svg_cairo_end_element (void *closure);

static svg_status_t
_svg_cairo_end_group (void *closure, double opacity);

static svg_status_t
_svg_cairo_move_to (void *closure, double x, double y);

static svg_status_t
_svg_cairo_line_to (void *closure, double x, double y);

static svg_status_t
_svg_cairo_curve_to (void *closure,
		     double x1, double y1,
		     double x2, double y2,
		     double x3, double y3);

static svg_status_t
_svg_cairo_quadratic_curve_to (void *closure,
			       double x1, double y1,
			       double x2, double y2);

static svg_status_t
_svg_cairo_arc_to (void	       *closure,
		   double	rx,
		   double	ry,
		   double	x_axis_rotation,
		   int		large_arc_flag,
		   int		sweep_flag,
		   double	x,
		   double	y);

static void
_svg_path_arc_segment (cairo_t *cr,
		       double   xc,  double yc,
		       double   th0, double th1,
		       double   rx,  double ry,
		       double   x_axis_rotation);

static svg_status_t
_svg_cairo_close_path (void *closure);

static svg_status_t
_svg_cairo_set_color (void *closure, const svg_color_t *color);

static svg_status_t
_svg_cairo_set_fill_opacity (void *closure, double fill_opacity);

static svg_status_t
_svg_cairo_set_fill_paint (void *closure, const svg_paint_t *paint);

static svg_status_t
_svg_cairo_set_fill_rule (void *closure, svg_fill_rule_t fill_rule);

static svg_status_t
_svg_cairo_set_font_family (void *closure, const char *family);

static svg_status_t
_svg_cairo_set_font_size (void *closure, svg_length_t *size);

static svg_status_t
_svg_cairo_set_font_style (void *closure, svg_font_style_t font_style);

static svg_status_t
_svg_cairo_set_font_weight (void *closure, unsigned int weight);

static svg_status_t
_svg_cairo_set_opacity (void *closure, double opacity);

static svg_status_t
_svg_cairo_set_stroke_dash_array (void *closure, double *dash, int num_dashes);

static svg_status_t
_svg_cairo_set_stroke_dash_offset (void *closure, svg_length_t *offset);

static svg_status_t
_svg_cairo_set_stroke_line_cap (void *closure, svg_stroke_line_cap_t line_cap);

static svg_status_t
_svg_cairo_set_stroke_line_join (void *closure, svg_stroke_line_join_t line_join);

static svg_status_t
_svg_cairo_set_stroke_miter_limit (void *closure, double limit);

static svg_status_t
_svg_cairo_set_stroke_opacity (void *closure, double stroke_opacity);

static svg_status_t
_svg_cairo_set_stroke_paint (void *closure, const svg_paint_t *paint);

static svg_status_t
_svg_cairo_set_stroke_width (void *closure, svg_length_t *width);

static svg_status_t
_svg_cairo_set_text_anchor (void *closure, svg_text_anchor_t text_anchor);

static svg_status_t
_svg_cairo_get_text_anchor (void *closure, svg_text_anchor_t *text_anchor);

static svg_status_t
_svg_cairo_transform (void *closure,
		      double a, double b,
		      double c, double d,
		      double e, double f);

static svg_status_t
_svg_cairo_apply_view_box (void *closure,
			   svg_view_box_t view_box,
			   svg_length_t *width,
			   svg_length_t *height);

static svg_status_t
_svg_cairo_set_viewport_dimension (void *closure,
				   svg_length_t *width,
				   svg_length_t *height);

static svg_status_t
_svg_cairo_render_line (void *closure,
			svg_length_t *x1_len, svg_length_t *y1_len,
			svg_length_t *x2_len, svg_length_t *y2_len);

static svg_status_t
_svg_cairo_render_path (void *closure);

static svg_status_t
_svg_cairo_render_ellipse (void *closure,
			   svg_length_t *cx,
			   svg_length_t *cy,
			   svg_length_t *rx,
			   svg_length_t *ry);

static svg_status_t
_svg_cairo_render_rect (void 	     *closure,
			svg_length_t *x,
			svg_length_t *y,
			svg_length_t *width,
			svg_length_t *height,
			svg_length_t *rx,
			svg_length_t *ry);

static svg_status_t
_svg_cairo_render_text (void 	      *closure,
			double	      x,
			double	      y,
			const char    *utf8);

static svg_status_t
_svg_cairo_render_image (void		*closure,
			 const char 	*url,
			 svg_length_t 	*x,
			 svg_length_t 	*y,
			 svg_length_t 	*width,
			 svg_length_t 	*height);

static svg_status_t
_svg_cairo_text_extents (void	      *closure,
			 const char   *utf8,
			 double	      *x,
			 double	      *y);

static svg_status_t
_svg_cairo_measure_position (void	    *closure,
			     svg_length_t   *ix,
			     svg_length_t   *iy,
			     double	    *ox,
			     double	    *oy);
			   
static svg_status_t
_cairo_status_to_svg_status (cairo_status_t xr_status);

static svg_status_t
_svg_cairo_push_state (svg_cairo_t     *svg_cairo,
		       cairo_surface_t *child_surface);

static svg_status_t
_svg_cairo_pop_state (svg_cairo_t *svg_cairo);

static svg_status_t
_svg_cairo_length_to_pixel (svg_cairo_t *svg_cairo, svg_length_t *length, double *pixel);

static svg_render_engine_t SVG_CAIRO_RENDER_ENGINE = {
    /* hierarchy */
    _svg_cairo_begin_group,
    _svg_cairo_begin_element,
    _svg_cairo_end_element,
    _svg_cairo_end_group,
    /* path creation */
    _svg_cairo_move_to,
    _svg_cairo_line_to,
    _svg_cairo_curve_to,
    _svg_cairo_quadratic_curve_to,
    _svg_cairo_arc_to,
    _svg_cairo_close_path,
    /* style */
    _svg_cairo_set_color,
    _svg_cairo_set_fill_opacity,
    _svg_cairo_set_fill_paint,
    _svg_cairo_set_fill_rule,
    _svg_cairo_set_font_family,
    _svg_cairo_set_font_size,
    _svg_cairo_set_font_style,
    _svg_cairo_set_font_weight,
    _svg_cairo_set_opacity,
    _svg_cairo_set_stroke_dash_array,
    _svg_cairo_set_stroke_dash_offset,
    _svg_cairo_set_stroke_line_cap,
    _svg_cairo_set_stroke_line_join,
    _svg_cairo_set_stroke_miter_limit,
    _svg_cairo_set_stroke_opacity,
    _svg_cairo_set_stroke_paint,
    _svg_cairo_set_stroke_width,
    _svg_cairo_set_text_anchor,
    /* transform */
    _svg_cairo_transform,
    _svg_cairo_apply_view_box,
    _svg_cairo_set_viewport_dimension,
    /* drawing */
    _svg_cairo_render_line,
    _svg_cairo_render_path,
    _svg_cairo_render_ellipse,
    _svg_cairo_render_rect,
    _svg_cairo_render_text,
    _svg_cairo_render_image,
    _svg_cairo_text_extents,
    _svg_cairo_measure_position,
    _svg_cairo_get_text_anchor,
};

svg_cairo_status_t
svg_cairo_create (svg_cairo_t **svg_cairo)
{
    svg_cairo_status_t status;

    *svg_cairo = malloc (sizeof (svg_cairo_t));
    if (*svg_cairo == NULL) {
	return SVG_CAIRO_STATUS_NO_MEMORY;
    }

    (*svg_cairo)->cr = NULL;
    (*svg_cairo)->state = NULL;
    /* XXX: These arbitrary constants don't belong. The viewport
     * handling should be reworked. */
    (*svg_cairo)->viewport_width = 450;
    (*svg_cairo)->viewport_height = 450;
 
    status = svg_create (&(*svg_cairo)->svg);
    if (status)
	return status;

    _svg_cairo_push_state (*svg_cairo, NULL);

    return SVG_CAIRO_STATUS_SUCCESS;
}

svg_cairo_status_t
svg_cairo_destroy (svg_cairo_t *svg_cairo)
{
    svg_cairo_status_t status;

    _svg_cairo_pop_state (svg_cairo);

    status = svg_destroy (svg_cairo->svg);

    free (svg_cairo);

    return status;
}

svg_cairo_status_t
svg_cairo_parse (svg_cairo_t *svg_cairo, const char *filename)
{
    return svg_parse (svg_cairo->svg, filename);
}

svg_cairo_status_t
svg_cairo_parse_file (svg_cairo_t *svg_cairo, FILE *file)
{
    return svg_parse_file (svg_cairo->svg, file);
}

svg_cairo_status_t
svg_cairo_parse_buffer (svg_cairo_t *svg_cairo, const char *buf, size_t count)
{
    return svg_parse_buffer (svg_cairo->svg, buf, count);
}

svg_cairo_status_t
svg_cairo_parse_chunk_begin (svg_cairo_t *svg_cairo)
{
    return svg_parse_chunk_begin (svg_cairo->svg);
}

svg_cairo_status_t
svg_cairo_parse_chunk (svg_cairo_t *svg_cairo, const char *buf, size_t count)
{
    return svg_parse_chunk (svg_cairo->svg, buf, count);
}

svg_cairo_status_t
svg_cairo_parse_chunk_end (svg_cairo_t *svg_cairo)
{
    return svg_parse_chunk_end (svg_cairo->svg);
}

svg_cairo_status_t
svg_cairo_render (svg_cairo_t *svg_cairo, cairo_t *cr)
{
    svg_cairo->cr = cr;
    return svg_render (svg_cairo->svg, &SVG_CAIRO_RENDER_ENGINE, svg_cairo);
}

svg_cairo_status_t
svg_cairo_render_2 (svg_cairo_t *svg_cairo, svg_t *svg, cairo_t *cr)
{
    svg_t *orig_svg = svg_cairo->svg;
    svg_cairo_status_t status;
    
    svg_cairo->cr = cr;
    if (svg)
	svg_cairo->svg = svg;
    
    status = svg_render (svg, &SVG_CAIRO_RENDER_ENGINE, svg_cairo);
    
    svg_cairo->svg = orig_svg;
    
    return status;
}

svg_cairo_status_t
svg_cairo_render_element (svg_cairo_t *svg_cairo, 
			  svg_t	*svg, 
			  svg_element_t *element,
			  cairo_t *cr)
{
    svg_t *orig_svg = svg_cairo->svg;
    svg_cairo_status_t status;
    
    svg_cairo->cr = cr;
    if (svg)
	svg_cairo->svg = svg;
    
    status = svg_element_render (element, &SVG_CAIRO_RENDER_ENGINE, svg_cairo);
    
    svg_cairo->svg = orig_svg;
    
    return status;
}

svg_cairo_status_t
svg_cairo_set_drawing_context (svg_cairo_t *svg_cairo, cairo_t *cr)
{
    svg_cairo->cr = cr;

    return SVG_CAIRO_STATUS_SUCCESS;
}

static svg_status_t
_svg_cairo_set_viewport_dimension (void *closure,
		    	      svg_length_t *width,
		    	      svg_length_t *height)
{
    svg_cairo_t *svg_cairo = closure;
    double vwidth, vheight;

    _svg_cairo_length_to_pixel (svg_cairo, width, &vwidth);
    _svg_cairo_length_to_pixel (svg_cairo, height, &vheight);

    /* TODO : change svg_cairo->state->viewport_width to type double
       if that accuracy is needed. */
    svg_cairo->state->viewport_width  = (unsigned int)vwidth;
    svg_cairo->state->viewport_height = (unsigned int)vheight;

    return SVG_CAIRO_STATUS_SUCCESS;
}

svg_cairo_status_t
svg_cairo_set_viewport_dimension (svg_cairo_t *svg_cairo, unsigned int width, unsigned int height)
{
    svg_cairo->viewport_width = width;
    svg_cairo->viewport_height = height;

    return SVG_CAIRO_STATUS_SUCCESS;
}

void
svg_cairo_get_size (svg_cairo_t  *svg_cairo,
		    unsigned int *width,
		    unsigned int *height)
{
    svg_length_t width_len, height_len;
    double width_d, height_d;

    svg_get_size (svg_cairo->svg, &width_len, &height_len);
    _svg_cairo_length_to_pixel (svg_cairo, &width_len, &width_d);
    _svg_cairo_length_to_pixel (svg_cairo, &height_len, &height_d);
    *width = (unsigned int) (width_d + 0.5);
    *height = (unsigned int) (height_d + 0.5);
}

void
svg_cairo_get_render_engine (svg_cairo_t *svg_cairo, svg_render_engine_t **engine, void **closure)
{
    *engine = &SVG_CAIRO_RENDER_ENGINE;
    *closure = svg_cairo;
}

static svg_status_t
_svg_cairo_begin_group (void *closure, double opacity, const char *id, const char *klass)
{
    svg_cairo_t *svg_cairo = closure;
    cairo_surface_t *child_surface = NULL;

    cairo_save (svg_cairo->cr);

    if (opacity != 1.0) {
	child_surface = cairo_surface_create_similar (cairo_get_target (svg_cairo->cr),
						      CAIRO_CONTENT_COLOR_ALPHA,
						      svg_cairo->state->viewport_width,
						      svg_cairo->state->viewport_height);
	svg_cairo->state->child_surface = child_surface;
    }

    _svg_cairo_push_state (svg_cairo, child_surface);

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

/* XXX: begin_element could be made more efficient in that no extra
   group is needed if there is only one element in a group */
static svg_status_t
_svg_cairo_begin_element (void *closure, const char *id, const char *klass)
{
    svg_cairo_t *svg_cairo = closure;

    cairo_save (svg_cairo->cr);

    _svg_cairo_push_state (svg_cairo, NULL);

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

static svg_status_t
_svg_cairo_end_element (void *closure)
{
    svg_cairo_t *svg_cairo = closure;

    _svg_cairo_pop_state (svg_cairo);

    cairo_restore (svg_cairo->cr);

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

static svg_status_t
_svg_cairo_end_group (void *closure, double opacity)
{
    svg_cairo_t *svg_cairo = closure;

    _svg_cairo_pop_state (svg_cairo);

    cairo_restore (svg_cairo->cr);

    if (opacity != 1.0) {
	cairo_save (svg_cairo->cr);
	cairo_identity_matrix (svg_cairo->cr);
	cairo_set_source_surface (svg_cairo->cr, svg_cairo->state->child_surface, 0, 0);
	cairo_paint_with_alpha (svg_cairo->cr, opacity);
	cairo_restore (svg_cairo->cr);
	cairo_surface_destroy (svg_cairo->state->child_surface);
	svg_cairo->state->child_surface = NULL;
    }

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

static svg_status_t
_svg_cairo_move_to (void *closure, double x, double y)
{
    svg_cairo_t *svg_cairo = closure;

    cairo_move_to (svg_cairo->cr, x, y);

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

static svg_status_t
_svg_cairo_line_to (void *closure, double x, double y)
{
    svg_cairo_t *svg_cairo = closure;

    cairo_line_to (svg_cairo->cr, x, y);

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

static svg_status_t
_svg_cairo_curve_to (void *closure,
		double x1, double y1,
		double x2, double y2,
		double x3, double y3)
{
    svg_cairo_t *svg_cairo = closure;

    cairo_curve_to (svg_cairo->cr,
		    x1, y1,
		    x2, y2,
		    x3, y3);

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

static svg_status_t
_svg_cairo_quadratic_curve_to (void *closure,
			  double x1, double y1,
			  double x2, double y2)
{
    svg_cairo_t *svg_cairo = closure;
    double x, y;

    cairo_get_current_point (svg_cairo->cr, &x, &y);

    cairo_curve_to (svg_cairo->cr,
		    x  + 2.0/3.0 * (x1 - x),  y  + 2.0/3.0 * (y1 - y),
		    x2 + 2.0/3.0 * (x1 - x2), y2 + 2.0/3.0 * (y1 - y2),
		    x2, y2);

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

static svg_status_t
_svg_cairo_close_path (void *closure)
{
    svg_cairo_t *svg_cairo = closure;

    cairo_close_path (svg_cairo->cr);

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

static svg_status_t
_svg_cairo_set_color (void *closure, const svg_color_t *color)
{
    svg_cairo_t *svg_cairo = closure;

    svg_cairo->state->color = *color;

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

static svg_status_t
_svg_cairo_set_color_and_alpha (svg_cairo_t *svg_cairo,
				svg_color_t *color,
				double alpha)
{
    if (color->is_current_color)
	color = &svg_cairo->state->color;

    cairo_set_source_rgba (svg_cairo->cr,
			   svg_color_get_red   (color) / 255.0,
			   svg_color_get_green (color) / 255.0,
			   svg_color_get_blue  (color) / 255.0,
			   svg_cairo->state->opacity * alpha);

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

static svg_status_t
_svg_cairo_set_gradient (svg_cairo_t *svg_cairo,
			 svg_gradient_t *gradient,
			 svg_cairo_render_type_t type)
{
    svg_gradient_stop_t *stop;
    cairo_pattern_t *pattern = NULL;
    cairo_matrix_t matrix, gradient_matrix;
    int i;

    cairo_matrix_init_identity (&matrix);

    switch (gradient->units) {
    case SVG_GRADIENT_UNITS_USER:
	break;
    case SVG_GRADIENT_UNITS_BBOX:
    {
	double x1, y1, x2, y2;
	
	if (type == SVG_CAIRO_RENDER_TYPE_FILL)
	    cairo_fill_extents (svg_cairo->cr, &x1, &y1, &x2, &y2);
	else
	    cairo_stroke_extents (svg_cairo->cr, &x1, &y1, &x2, &y2);

	cairo_matrix_translate (&matrix, x1, y1);
	cairo_matrix_scale (&matrix, x2 - x1, y2 - y1);
	svg_cairo->state->bbox = 1;
    } break;
    }
    
    switch (gradient->type) {
    case SVG_GRADIENT_LINEAR:
    {
	double x1, y1, x2, y2;
      
	_svg_cairo_length_to_pixel (svg_cairo, &gradient->u.linear.x1, &x1);
	_svg_cairo_length_to_pixel (svg_cairo, &gradient->u.linear.y1, &y1);
	_svg_cairo_length_to_pixel (svg_cairo, &gradient->u.linear.x2, &x2);
	_svg_cairo_length_to_pixel (svg_cairo, &gradient->u.linear.y2, &y2);

	pattern = cairo_pattern_create_linear (x1, y1, x2, y2);
    }
    break;
    case SVG_GRADIENT_RADIAL:
    {
	double cx, cy, r, fx, fy;
      
	_svg_cairo_length_to_pixel (svg_cairo, &gradient->u.radial.cx, &cx);
	_svg_cairo_length_to_pixel (svg_cairo, &gradient->u.radial.cy, &cy);
	_svg_cairo_length_to_pixel (svg_cairo, &gradient->u.radial.r, &r);
	_svg_cairo_length_to_pixel (svg_cairo, &gradient->u.radial.fx, &fx);
	_svg_cairo_length_to_pixel (svg_cairo, &gradient->u.radial.fy, &fy);

	pattern = cairo_pattern_create_radial (fx, fy, 0.0, cx, cy, r);
    } break;
    }
    
    for (i = 0; i < gradient->num_stops; i++) {
	stop = &gradient->stops[i];
	cairo_pattern_add_color_stop_rgba (pattern, stop->offset,
					   svg_color_get_red (&stop->color) / 255.0,
					   svg_color_get_green (&stop->color) / 255.0,
					   svg_color_get_blue (&stop->color) / 255.0,
					   svg_cairo->state->opacity * stop->opacity);
    }
	    
    switch (gradient->spread) {
    case SVG_GRADIENT_SPREAD_REPEAT:
	cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REPEAT);
	break;
    case SVG_GRADIENT_SPREAD_REFLECT:
	cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REFLECT);
	break;
    default:
	cairo_pattern_set_extend (pattern, CAIRO_EXTEND_PAD);
	break;
    }
    
    cairo_pattern_set_filter (pattern, CAIRO_FILTER_BILINEAR);

    cairo_matrix_init (&gradient_matrix,
		       gradient->transform[0], gradient->transform[1],
		       gradient->transform[2], gradient->transform[3],
		       gradient->transform[4], gradient->transform[5]);
    cairo_matrix_multiply (&matrix, &matrix, &gradient_matrix);
    
    cairo_matrix_invert (&matrix);
    cairo_pattern_set_matrix (pattern, &matrix);
    
    cairo_set_source (svg_cairo->cr, pattern);
    cairo_pattern_destroy (pattern);
    svg_cairo->state->bbox = 0;
    
    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_cairo_set_pattern (svg_cairo_t *svg_cairo,
			svg_element_t *pattern_element,
			svg_cairo_render_type_t type)
{
    svg_pattern_t *pattern = svg_element_pattern (pattern_element);
    cairo_surface_t *pattern_surface;
    cairo_pattern_t *surface_pattern;
    double x_px, y_px, width_px, height_px;
    cairo_path_t *path;

    _svg_cairo_length_to_pixel (svg_cairo, &pattern->x, &x_px);
    _svg_cairo_length_to_pixel (svg_cairo, &pattern->y, &y_px);
    _svg_cairo_length_to_pixel (svg_cairo, &pattern->width, &width_px);
    _svg_cairo_length_to_pixel (svg_cairo, &pattern->height, &height_px);

    /* OK. We've got the final path to be filled/stroked inside the
     * cairo context right now. But we're also going to re-use that
     * same context to draw the pattern. And since the path is no
     * longer in the graphics state, cairo_save/restore will not help
     * us here.
     *
     * Currently we deal with this by manually saving/restoring the
     * path.
     *
     * It might be simpler to just use a new cairo_t for drawing the
     * pattern.
     */
    path = cairo_copy_path (svg_cairo->cr);
    cairo_new_path (svg_cairo->cr);
    cairo_save (svg_cairo->cr);

    pattern_surface = cairo_surface_create_similar (cairo_get_target (svg_cairo->cr),
						    CAIRO_CONTENT_COLOR_ALPHA,
						    (int) (width_px + 0.5),
						    (int) (height_px + 0.5));

    _svg_cairo_push_state (svg_cairo, pattern_surface);
    cairo_identity_matrix (svg_cairo->cr);
    
    svg_cairo->state->fill_paint.type = SVG_PAINT_TYPE_NONE;
    svg_cairo->state->stroke_paint.type = SVG_PAINT_TYPE_NONE;
    
    svg_element_render (pattern->group_element, &SVG_CAIRO_RENDER_ENGINE, svg_cairo);
    _svg_cairo_pop_state (svg_cairo);

    cairo_restore (svg_cairo->cr);
    cairo_new_path (svg_cairo->cr);
    cairo_append_path (svg_cairo->cr, path);
    cairo_path_destroy (path);

    surface_pattern = cairo_pattern_create_for_surface (pattern_surface);
    cairo_surface_destroy (pattern_surface);
    
    cairo_pattern_set_extend (surface_pattern, CAIRO_EXTEND_REPEAT);
    
    cairo_set_source (svg_cairo->cr, surface_pattern);
    
    cairo_pattern_destroy (surface_pattern);

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_cairo_set_paint_and_opacity (svg_cairo_t *svg_cairo, svg_paint_t *paint, double opacity, svg_cairo_render_type_t type)
{
    svg_status_t status;

    switch (paint->type) {
    case SVG_PAINT_TYPE_NONE:
	break;
    case SVG_PAINT_TYPE_COLOR:
	status = _svg_cairo_set_color_and_alpha (svg_cairo,
						 &paint->p.color,
						 opacity);
	if (status)
	    return status;
	break;
    case SVG_PAINT_TYPE_GRADIENT:
	status = _svg_cairo_set_gradient (svg_cairo, paint->p.gradient, type);
	if (status)
	    return status;
	break;
    case SVG_PAINT_TYPE_PATTERN:
	status = _svg_cairo_set_pattern (svg_cairo, paint->p.pattern_element, type);
	if (status)
	    return status;
	break;
    }
    
    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_cairo_set_fill_opacity (void *closure, double fill_opacity)
{
    svg_cairo_t *svg_cairo = closure;

    svg_cairo->state->fill_opacity = fill_opacity;

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));    
}

static svg_status_t
_svg_cairo_set_fill_paint (void *closure, const svg_paint_t *paint)
{
    svg_cairo_t *svg_cairo = closure;

    svg_cairo->state->fill_paint = *paint;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_cairo_set_fill_rule (void *closure, svg_fill_rule_t fill_rule)
{
    svg_cairo_t *svg_cairo = closure;

    switch (fill_rule) {
    case SVG_FILL_RULE_NONZERO:
	cairo_set_fill_rule (svg_cairo->cr, CAIRO_FILL_RULE_WINDING);
	break;
    case SVG_FILL_RULE_EVEN_ODD:
	cairo_set_fill_rule (svg_cairo->cr, CAIRO_FILL_RULE_EVEN_ODD);
	break;
    }

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));    
}

#if !HAVE_PANGOCAIRO
static svg_status_t
_svg_cairo_select_font (svg_cairo_t *svg_cairo)
{
    char *family = svg_cairo->state->font_family;
    unsigned int font_weight = svg_cairo->state->font_weight;
    cairo_font_weight_t weight;
    svg_font_style_t font_style = svg_cairo->state->font_style;
    cairo_font_slant_t slant;

    if (! svg_cairo->state->font_dirty)
	return SVG_STATUS_SUCCESS;

    if (font_weight >= 700)
	weight = CAIRO_FONT_WEIGHT_BOLD;
    else
	weight = CAIRO_FONT_WEIGHT_NORMAL;

    switch (font_style) {
    case SVG_FONT_STYLE_ITALIC:
	slant = CAIRO_FONT_SLANT_ITALIC;
	break;
    case SVG_FONT_STYLE_OBLIQUE:
	slant = CAIRO_FONT_SLANT_OBLIQUE;
	break;
    default:
	slant = CAIRO_FONT_SLANT_NORMAL;
	break;
    }

    cairo_select_font_face (svg_cairo->cr, family, slant, weight);
    cairo_set_font_size (svg_cairo->cr, svg_cairo->state->font_size);
    svg_cairo->state->font_dirty = 0;

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}
#endif

static svg_status_t
_svg_cairo_set_font_family (void *closure, const char *family)
{
    svg_cairo_t *svg_cairo = closure;

#if HAVE_PANGOCAIRO
    pango_font_description_set_family (svg_cairo->state->font_description,
				       family);
#else
    if (svg_cairo->state->font_family)
	free (svg_cairo->state->font_family);

    svg_cairo->state->font_family = strdup (family);
    svg_cairo->state->font_dirty = 1;
#endif

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

static svg_status_t
_svg_cairo_set_font_size (void *closure, svg_length_t *size)
{
    svg_cairo_t *svg_cairo = closure;
    double font_size;

    _svg_cairo_length_to_pixel (svg_cairo, size, &font_size);

#if HAVE_PANGOCAIRO
    pango_font_description_set_absolute_size (svg_cairo->state->font_description,
					      (int) (font_size * PANGO_SCALE));
#else
    svg_cairo->state->font_size = font_size;
    svg_cairo->state->font_dirty = 1;
#endif

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

static svg_status_t
_svg_cairo_set_font_style (void *closure, svg_font_style_t font_style)
{
    svg_cairo_t *svg_cairo = closure;
    
#if HAVE_PANGOCAIRO
    PangoStyle	style;

    switch (font_style) {
    case SVG_FONT_STYLE_ITALIC:
	style = PANGO_STYLE_ITALIC;
	break;
    case SVG_FONT_STYLE_OBLIQUE:
	style = PANGO_STYLE_OBLIQUE;
	break;
    default:
	style = PANGO_STYLE_NORMAL;
	break;
    }
	
    pango_font_description_set_style (svg_cairo->state->font_description,
				      style);
#else
    svg_cairo->state->font_style = font_style;
    svg_cairo->state->font_dirty = 1;
#endif

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

static svg_status_t
_svg_cairo_set_font_weight (void *closure, unsigned int font_weight)
{
    svg_cairo_t *svg_cairo = closure;

#if HAVE_PANGOCAIRO
    /* Pango weights are the same as SVG weights */
    pango_font_description_set_weight (svg_cairo->state->font_description,
				       font_weight);
#else
    svg_cairo->state->font_weight = font_weight;
    svg_cairo->state->font_dirty = 1;
#endif

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

static svg_status_t
_svg_cairo_set_opacity (void *closure, double opacity)
{
    svg_cairo_t *svg_cairo = closure;

    svg_cairo->state->opacity = opacity;

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));    
}

static svg_status_t
_svg_cairo_set_stroke_dash_array (void *closure, double *dash, int num_dashes)
{
    svg_cairo_t *svg_cairo = closure;

    free (svg_cairo->state->dash);
    svg_cairo->state->dash = NULL;

    svg_cairo->state->num_dashes = num_dashes;

    if (svg_cairo->state->num_dashes) {
	svg_cairo->state->dash = malloc(svg_cairo->state->num_dashes * sizeof(double));
	if (svg_cairo->state->dash == NULL)
	    return SVG_STATUS_NO_MEMORY;

	memcpy(svg_cairo->state->dash, dash, svg_cairo->state->num_dashes * sizeof(double));

	cairo_set_dash (svg_cairo->cr, svg_cairo->state->dash, svg_cairo->state->num_dashes, svg_cairo->state->dash_offset);
    }

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

static svg_status_t
_svg_cairo_set_stroke_dash_offset (void *closure, svg_length_t *offset_len)
{
    svg_cairo_t *svg_cairo = closure;
    double offset;

    _svg_cairo_length_to_pixel (svg_cairo, offset_len, &offset);

    svg_cairo->state->dash_offset = offset;

    if (svg_cairo->state->num_dashes)
	cairo_set_dash (svg_cairo->cr, svg_cairo->state->dash, svg_cairo->state->num_dashes, svg_cairo->state->dash_offset); 
    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

static svg_status_t
_svg_cairo_set_stroke_line_cap (void *closure, svg_stroke_line_cap_t line_cap)
{
    svg_cairo_t *svg_cairo = closure;

    switch (line_cap) {
    case SVG_STROKE_LINE_CAP_BUTT:
	cairo_set_line_cap (svg_cairo->cr, CAIRO_LINE_CAP_BUTT);
	break;
    case SVG_STROKE_LINE_CAP_ROUND:
	cairo_set_line_cap (svg_cairo->cr, CAIRO_LINE_CAP_ROUND);
	break;
    case SVG_STROKE_LINE_CAP_SQUARE:
	cairo_set_line_cap (svg_cairo->cr, CAIRO_LINE_CAP_SQUARE);
	break;
    }

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

static svg_status_t
_svg_cairo_set_stroke_line_join (void *closure, svg_stroke_line_join_t line_join)
{
    svg_cairo_t *svg_cairo = closure;

    switch (line_join) {
    case SVG_STROKE_LINE_JOIN_MITER:
	cairo_set_line_join (svg_cairo->cr, CAIRO_LINE_JOIN_MITER);
	break;
    case SVG_STROKE_LINE_JOIN_ROUND:
	cairo_set_line_join (svg_cairo->cr, CAIRO_LINE_JOIN_ROUND);
	break;
    case SVG_STROKE_LINE_JOIN_BEVEL:
	cairo_set_line_join (svg_cairo->cr, CAIRO_LINE_JOIN_BEVEL);
	break;
    }

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

static svg_status_t
_svg_cairo_set_stroke_miter_limit (void *closure, double limit)
{
    svg_cairo_t *svg_cairo = closure;

    cairo_set_miter_limit (svg_cairo->cr, limit);

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));    
}

static svg_status_t
_svg_cairo_set_stroke_opacity (void *closure, double stroke_opacity)
{
    svg_cairo_t *svg_cairo = closure;

    svg_cairo->state->stroke_opacity = stroke_opacity;

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));    
}

static svg_status_t
_svg_cairo_set_stroke_paint (void *closure, const svg_paint_t *paint)
{
    svg_cairo_t *svg_cairo = closure;

    svg_cairo->state->stroke_paint = *paint;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_cairo_set_stroke_width (void *closure, svg_length_t *width_len)
{
    svg_cairo_t *svg_cairo = closure;
    double width;

    _svg_cairo_length_to_pixel (svg_cairo, width_len, &width);

    cairo_set_line_width (svg_cairo->cr, width);

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));    
}

static svg_status_t
_svg_cairo_set_text_anchor (void *closure, svg_text_anchor_t text_anchor)
{
    svg_cairo_t *svg_cairo = closure;

    svg_cairo->state->text_anchor = text_anchor;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_cairo_get_text_anchor (void *closure, svg_text_anchor_t *text_anchor)
{
    svg_cairo_t *svg_cairo = closure;

    *text_anchor = svg_cairo->state->text_anchor;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_cairo_transform (void *closure,
		  double a, double b,
		  double c, double d,
		  double e, double f)
{
    svg_cairo_t *svg_cairo = closure;
    cairo_matrix_t matrix;

    cairo_matrix_init (&matrix, a, b, c, d, e, f);
    cairo_transform (svg_cairo->cr, &matrix);

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));    
}

static svg_status_t
_svg_cairo_render_line (void *closure,
		   svg_length_t *x1_len, svg_length_t *y1_len,
		   svg_length_t *x2_len, svg_length_t *y2_len)
{
    svg_cairo_t *svg_cairo = closure;
    svg_status_t status;
    double x1, y1, x2, y2;

    _svg_cairo_length_to_pixel (svg_cairo, x1_len, &x1);
    _svg_cairo_length_to_pixel (svg_cairo, y1_len, &y1);
    _svg_cairo_length_to_pixel (svg_cairo, x2_len, &x2);
    _svg_cairo_length_to_pixel (svg_cairo, y2_len, &y2);

    status = _svg_cairo_move_to (svg_cairo, x1, y1);
    if (status)
	return status;

    status = _svg_cairo_line_to (svg_cairo, x2, y2);
    if (status)
	return status;

    status = _svg_cairo_render_path (svg_cairo);
    if (status)
	return status;

    return SVG_STATUS_SUCCESS;

}

static svg_status_t
_svg_cairo_render_path (void *closure)
{
    svg_cairo_t *svg_cairo = closure;
    svg_paint_t *fill_paint, *stroke_paint;

    fill_paint = &svg_cairo->state->fill_paint;
    stroke_paint = &svg_cairo->state->stroke_paint;

    if (fill_paint->type) {
	_svg_cairo_set_paint_and_opacity (svg_cairo, fill_paint,
					  svg_cairo->state->fill_opacity,
					  SVG_CAIRO_RENDER_TYPE_FILL);
	if (stroke_paint->type)
	    cairo_fill_preserve (svg_cairo->cr);
	else
	    cairo_fill (svg_cairo->cr);
    }

    if (stroke_paint->type) {
	_svg_cairo_set_paint_and_opacity (svg_cairo, stroke_paint,
					  svg_cairo->state->stroke_opacity,
					  SVG_CAIRO_RENDER_TYPE_STROKE);
	cairo_stroke (svg_cairo->cr);
    }

    /* This is only strictly necessary in the odd case of both
     * fill_paint->type and stroke_paint->type being false. But it's
     * easier, and doesn't hurt much to just do it here
     * unconditionally. */
    cairo_new_path (svg_cairo->cr);

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

static svg_status_t
_svg_cairo_render_ellipse (void *closure,
		      svg_length_t *cx_len,
		      svg_length_t *cy_len,
		      svg_length_t *rx_len,
		      svg_length_t *ry_len)
{
    svg_cairo_t *svg_cairo = closure;
    cairo_matrix_t matrix;

    double cx, cy, rx, ry;

    _svg_cairo_length_to_pixel (svg_cairo, cx_len, &cx);
    _svg_cairo_length_to_pixel (svg_cairo, cy_len, &cy);
    _svg_cairo_length_to_pixel (svg_cairo, rx_len, &rx);
    _svg_cairo_length_to_pixel (svg_cairo, ry_len, &ry);

    cairo_get_matrix (svg_cairo->cr, &matrix);

    cairo_translate (svg_cairo->cr, cx, cy);
    cairo_scale (svg_cairo->cr, 1.0, ry / rx);
    cairo_move_to (svg_cairo->cr, rx, 0.0);
    cairo_arc (svg_cairo->cr, 0, 0, rx, 0, 2 * M_PI);
    cairo_close_path (svg_cairo->cr);

    cairo_set_matrix (svg_cairo->cr, &matrix);

     _svg_cairo_render_path (svg_cairo);

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

static svg_status_t
_svg_cairo_render_rect (void *closure,
		   svg_length_t *x_len,
		   svg_length_t *y_len,
		   svg_length_t *width_len,
		   svg_length_t *height_len,
		   svg_length_t *rx_len,
		   svg_length_t *ry_len)
{
    svg_cairo_t *svg_cairo = closure;

    double x, y, width, height, rx, ry;
 
    _svg_cairo_length_to_pixel (svg_cairo, x_len, &x);
    _svg_cairo_length_to_pixel (svg_cairo, y_len, &y);
    _svg_cairo_length_to_pixel (svg_cairo, width_len, &width);
    _svg_cairo_length_to_pixel (svg_cairo, height_len, &height);
    _svg_cairo_length_to_pixel (svg_cairo, rx_len, &rx);
    _svg_cairo_length_to_pixel (svg_cairo, ry_len, &ry);
 
    if (rx > width / 2.0)
	rx = width / 2.0;
    if (ry > height / 2.0)
	ry = height / 2.0;

    if (rx > 0 || ry > 0)
    {
	_svg_cairo_move_to (svg_cairo, x + rx, y);
	_svg_cairo_line_to (svg_cairo, x + width - rx, y);
	_svg_cairo_arc_to  (svg_cairo, rx, ry, 0, 0, 1, x + width, y + ry);
	_svg_cairo_line_to (svg_cairo, x + width, y + height - ry);
	_svg_cairo_arc_to  (svg_cairo, rx, ry, 0, 0, 1, x + width - rx, y + height);
	_svg_cairo_line_to (svg_cairo, x + rx, y + height);
	_svg_cairo_arc_to  (svg_cairo, rx, ry, 0, 0, 1, x, y + height - ry);
	_svg_cairo_line_to (svg_cairo, x, y + ry);
	_svg_cairo_arc_to  (svg_cairo, rx, ry, 0, 0, 1, x + rx, y);
    }
    else
    {
	_svg_cairo_move_to (svg_cairo, x, y);
	_svg_cairo_line_to (svg_cairo, x + width, y);
	_svg_cairo_line_to (svg_cairo, x + width, y + height);
	_svg_cairo_line_to (svg_cairo, x, y + height);
    }
    _svg_cairo_close_path (svg_cairo);

    _svg_cairo_render_path (svg_cairo);

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

static svg_status_t
_svg_cairo_render_text (void *closure,
			double x,
			double y,
			const char *utf8)
{
    svg_cairo_t *svg_cairo = closure;
    svg_status_t status;
    svg_paint_t *fill_paint, *stroke_paint;
#if HAVE_PANGOCAIRO
    PangoLayout *layout;
    PangoLayoutLine *line;
#endif
    
    fill_paint = &svg_cairo->state->fill_paint;
    stroke_paint = &svg_cairo->state->stroke_paint;

    if (utf8 == NULL || *utf8 == '\0')
	return SVG_STATUS_SUCCESS;

    status = _svg_cairo_move_to (svg_cairo, x, y);
    if (status)
	return status;

#if HAVE_PANGOCAIRO
    layout = pango_cairo_create_layout (svg_cairo->cr);
    pango_layout_set_font_description (layout, svg_cairo->state->font_description);
    pango_layout_set_text (layout, utf8, -1);
    line = pango_layout_get_lines (layout)->data;
#else
    _svg_cairo_select_font (svg_cairo);
#endif

    if (fill_paint->type) {
	if (stroke_paint->type)
	    cairo_save (svg_cairo->cr);
	_svg_cairo_set_paint_and_opacity (svg_cairo, fill_paint,
					  svg_cairo->state->fill_opacity,
					  SVG_CAIRO_RENDER_TYPE_FILL);
	
#if HAVE_PANGOCAIRO
	pango_cairo_show_layout_line (svg_cairo->cr, line);
#else
	cairo_show_text (svg_cairo->cr, utf8);
#endif
	if (stroke_paint->type)
	    cairo_restore (svg_cairo->cr);
    }

    if (stroke_paint->type) {
	_svg_cairo_set_paint_and_opacity (svg_cairo, stroke_paint,
					  svg_cairo->state->stroke_opacity,
					  SVG_CAIRO_RENDER_TYPE_STROKE);
#if HAVE_PANGOCAIRO
	pango_cairo_layout_line_path (svg_cairo->cr, line);
#else
	cairo_text_path (svg_cairo->cr, utf8);
#endif
	cairo_stroke (svg_cairo->cr);
    }

#if HAVE_PANGOCAIRO
    g_object_unref (layout);
#endif

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

static svg_status_t
_svg_cairo_render_image (void  *closure,
			 const char *url,
			 svg_length_t *x_len,
			 svg_length_t *y_len,
			 svg_length_t *width_len,
			 svg_length_t *height_len)
{
    svg_cairo_t *svg_cairo = closure;
    svg_status_t status;
    cairo_surface_t *surface;
    double x, y, width, height;
    unsigned char* data;
    unsigned int data_width, data_height;
    
    status = svg_get_bgra_image (url, &data, &data_width, &data_height);
    if (status)
	return status;

    cairo_save (svg_cairo->cr);
    
    _svg_cairo_length_to_pixel (svg_cairo, x_len, &x);
    _svg_cairo_length_to_pixel (svg_cairo, y_len, &y);
    _svg_cairo_length_to_pixel (svg_cairo, width_len, &width);
    _svg_cairo_length_to_pixel (svg_cairo, height_len, &height);
    
    surface = cairo_image_surface_create_for_data ((unsigned char *)data, CAIRO_FORMAT_ARGB32,
	    data_width, data_height, data_width *4);
    cairo_translate (svg_cairo->cr, x, y);
    cairo_scale (svg_cairo->cr, width / data_width, height / data_height);
    
    cairo_set_source_surface (svg_cairo->cr, surface, 0, 0);
    if (svg_cairo->state->opacity != 1.0)
	cairo_paint_with_alpha (svg_cairo->cr, svg_cairo->state->opacity);
    else
	cairo_paint (svg_cairo->cr);
    
    cairo_surface_destroy (surface);
    
    cairo_restore (svg_cairo->cr);
    
    free (data);
    
    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}

static svg_status_t
_cairo_status_to_svg_status (cairo_status_t xr_status)
{
    switch (xr_status) {
    case CAIRO_STATUS_NO_MEMORY:
	return SVG_STATUS_NO_MEMORY;
    case CAIRO_STATUS_SUCCESS:
    default:
	return SVG_STATUS_SUCCESS;
    }
}

static svg_status_t
_svg_cairo_text_extents (void	      *closure,
			 const char   *utf8,
			 double	      *x,
			 double	      *y)
{
    svg_cairo_t *svg_cairo = closure;

#if HAVE_PANGOCAIRO
    PangoRectangle log;
    PangoLayout *layout;
    PangoLayoutLine *line;
    
    layout = pango_cairo_create_layout (svg_cairo->cr);
    pango_layout_set_font_description (layout, svg_cairo->state->font_description);
    pango_layout_set_text (layout, utf8, -1);
    line = pango_layout_get_lines (layout)->data;
    pango_layout_get_extents (layout, NULL, &log);
    *x = (double) log.width / PANGO_SCALE;
    *y = 0;
    g_object_unref (layout);
#else
    cairo_text_extents_t extents;
    
    _svg_cairo_select_font (svg_cairo);
    cairo_text_extents (svg_cairo->cr, utf8, &extents);
    *x = extents.x_advance;
    *y = extents.y_advance;
#endif
    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_cairo_measure_position (void	    *closure,
			     svg_length_t   *ix,
			     svg_length_t   *iy,
			     double	    *ox,
			     double	    *oy)
{
    svg_cairo_t *svg_cairo = closure;

    _svg_cairo_length_to_pixel (svg_cairo, ix, ox);
    _svg_cairo_length_to_pixel (svg_cairo, iy, oy);

    return SVG_STATUS_SUCCESS;
}
			   
static void
_svg_cairo_copy_cairo_state (svg_cairo_t *svg_cairo,
			     cairo_t     *old_cr,
			     cairo_t     *new_cr)
{
    cairo_matrix_t ctm;
    
    cairo_get_matrix (old_cr, &ctm);
    cairo_set_matrix (new_cr, &ctm);

    /* For simplicity copy all the state, some of these aren't needed,
     * since we don't change them.
     */
    cairo_set_operator (new_cr, cairo_get_operator (old_cr));
    cairo_set_source (new_cr, cairo_get_source (old_cr));
    cairo_set_tolerance (new_cr, cairo_get_tolerance (old_cr));
    cairo_set_fill_rule (new_cr, cairo_get_fill_rule (old_cr));
    cairo_set_line_width (new_cr, cairo_get_line_width (old_cr));
    cairo_set_line_cap (new_cr, cairo_get_line_cap (old_cr));
    cairo_set_line_join (new_cr, cairo_get_line_join (old_cr));
    cairo_set_miter_limit (new_cr, cairo_get_miter_limit (old_cr));

    /* There is no cairo_get_dash, but we already have a copy ourselves */
    cairo_set_dash (new_cr,
		    svg_cairo->state->dash, svg_cairo->state->num_dashes, svg_cairo->state->dash_offset);
}

static svg_status_t
_svg_cairo_push_state (svg_cairo_t     *svg_cairo,
		       cairo_surface_t *child_surface)
{
    if (!svg_cairo->state)
    {
	svg_cairo->state = _svg_cairo_state_push (svg_cairo->state);
	svg_cairo->state->viewport_width = svg_cairo->viewport_width;
	svg_cairo->state->viewport_height = svg_cairo->viewport_height;
    }
    else
    {
	if (child_surface)
	{
	    cairo_t *new_cr = cairo_create (child_surface);
	    if (!new_cr)
		return SVG_STATUS_NO_MEMORY;
	    
	    svg_cairo->state->saved_cr = svg_cairo->cr;
	    svg_cairo->cr = new_cr;
	    
	    _svg_cairo_copy_cairo_state (svg_cairo, svg_cairo->state->saved_cr, svg_cairo->cr);
	}
	svg_cairo->state = _svg_cairo_state_push (svg_cairo->state);
    }

    
    if (svg_cairo->state == NULL)
	return SVG_STATUS_NO_MEMORY;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_cairo_pop_state (svg_cairo_t *svg_cairo)
{
    svg_cairo->state = _svg_cairo_state_pop (svg_cairo->state);

    if (svg_cairo->state && svg_cairo->state->saved_cr) {
	cairo_destroy (svg_cairo->cr);
	svg_cairo->cr = svg_cairo->state->saved_cr;
	svg_cairo->state->saved_cr = NULL;
    }

    return SVG_STATUS_SUCCESS;
}

/* svg_get_dpi queries the current DPI but libsvg-cairo has no mechanism
   for setting this DPI.  Thus we have to assume that libsvg gets it right;
   there's no way for users to control it. */
#define	DPI(svg) svg_get_dpi(svg)

static double
_svg_cairo_text_size (svg_cairo_t *svg_cairo)
{
#if HAVE_PANGOCAIRO
    return ((double) pango_font_description_get_size (svg_cairo->state->font_description)
	    / PANGO_SCALE);
#else
    return svg_cairo->state->font_size;
#endif
}

static svg_status_t
_svg_cairo_length_to_pixel (svg_cairo_t * svg_cairo, svg_length_t *length, double *pixel)
{
    double width, height;

    switch (length->unit) {
    case SVG_LENGTH_UNIT_PX:
	*pixel = length->value;
	break;
    case SVG_LENGTH_UNIT_CM:
        *pixel = (length->value / 2.54) * DPI(svg_cairo->svg);
	break;
    case SVG_LENGTH_UNIT_MM:
	*pixel = (length->value / 25.4) * DPI(svg_cairo->svg);
	break;
    case SVG_LENGTH_UNIT_IN:
	*pixel = length->value * DPI(svg_cairo->svg);
	break;
    case SVG_LENGTH_UNIT_PT:
	*pixel = (length->value / 72.0) * DPI(svg_cairo->svg);
	break;
    case SVG_LENGTH_UNIT_PC:
	*pixel = (length->value / 6.0) * DPI(svg_cairo->svg);
	break;
    case SVG_LENGTH_UNIT_EM:
	*pixel = length->value * _svg_cairo_text_size (svg_cairo);
	break;
    case SVG_LENGTH_UNIT_EX:
	*pixel = length->value * _svg_cairo_text_size (svg_cairo) / 2.0;
	break;
    case SVG_LENGTH_UNIT_PCT:
	if (svg_cairo->state->bbox) {
	    width = 1.0;
	    height = 1.0;
	} else {
	    width = svg_cairo->state->viewport_width;
	    height = svg_cairo->state->viewport_height;
	}
	if (length->orientation == SVG_LENGTH_ORIENTATION_HORIZONTAL)
	    *pixel = (length->value / 100.0) * width;
	else if (length->orientation == SVG_LENGTH_ORIENTATION_VERTICAL)
	    *pixel = (length->value / 100.0) * height;
	else
	    *pixel = (length->value / 100.0) * sqrt(pow(width, 2) + pow(height, 2)) * sqrt(2);
	break;
    default:
	*pixel = length->value;
    }

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_cairo_apply_view_box (void *closure,
		      svg_view_box_t view_box,
		      svg_length_t *width,
		      svg_length_t *height)
{
    svg_cairo_t *svg_cairo = closure;
    double vpar, svgar;
    double logic_width, logic_height;
    double logic_x, logic_y;
    double phys_width, phys_height;
    _svg_cairo_length_to_pixel (svg_cairo, width, &phys_width);
    _svg_cairo_length_to_pixel (svg_cairo, height, &phys_height);

    vpar = view_box.box.width / view_box.box.height;
    svgar = phys_width / phys_height;
    logic_x = view_box.box.x;
    logic_y = view_box.box.y;
    logic_width = view_box.box.width;
    logic_height = view_box.box.height;

    if (view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_NONE)
    {
	cairo_scale (svg_cairo->cr,
		     phys_width / logic_width,
		     phys_height / logic_height);
	cairo_translate (svg_cairo->cr, -logic_x, -logic_y);
    }
    else if ((vpar < svgar && view_box.meet_or_slice == SVG_MEET_OR_SLICE_MEET) ||
    	     (vpar >= svgar && view_box.meet_or_slice == SVG_MEET_OR_SLICE_SLICE))
    {
	cairo_scale (svg_cairo->cr, phys_height / logic_height, phys_height / logic_height);

	if (view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMINYMIN ||
	    view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMINYMID ||
	    view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMINYMAX)
	    cairo_translate (svg_cairo->cr, -logic_x, -logic_y);
	else if(view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMIDYMIN ||
		view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMIDYMID ||
		view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMIDYMAX)
            cairo_translate (svg_cairo->cr,
			 -logic_x - (logic_width - phys_width * logic_height / phys_height) / 2,
			 -logic_y);
	else
            cairo_translate (svg_cairo->cr,
			 -logic_x - (logic_width - phys_width * logic_height / phys_height),
			 -logic_y);
    }
    else
    {
	cairo_scale (svg_cairo->cr, phys_width / logic_width, phys_width / logic_width);

	if (view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMINYMIN ||
	    view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMIDYMIN ||
	    view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMAXYMIN)
	    cairo_translate (svg_cairo->cr, -logic_x, -logic_y);
	else if(view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMINYMID ||
		view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMIDYMID ||
		view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMAXYMID)
	    cairo_translate (svg_cairo->cr,
			 -logic_x,
			 -logic_y - (logic_height - phys_height * logic_width / phys_width) / 2);
	else
	    cairo_translate (svg_cairo->cr,
			 -logic_x,
			 -logic_y - (logic_height - phys_height * logic_width / phys_width));
    }

    return SVG_STATUS_SUCCESS;
}

/* The ellipse and arc functions below are:
 
   Copyright (C) 2000 Eazel, Inc.
  
   Author: Raph Levien <raph@artofcode.com>

   This is adapted from svg-path in Gill.
*/
static void
_svg_path_arc_segment (cairo_t *cr,
		       double xc, double yc,
		       double th0, double th1,
		       double rx, double ry, double x_axis_rotation)
{
    double sin_th, cos_th;
    double a00, a01, a10, a11;
    double x1, y1, x2, y2, x3, y3;
    double t;
    double th_half;

    sin_th = sin (x_axis_rotation * (M_PI / 180.0));
    cos_th = cos (x_axis_rotation * (M_PI / 180.0)); 
    /* inverse transform compared with rsvg_path_arc */
    a00 = cos_th * rx;
    a01 = -sin_th * ry;
    a10 = sin_th * rx;
    a11 = cos_th * ry;

    th_half = 0.5 * (th1 - th0);
    t = (8.0 / 3.0) * sin (th_half * 0.5) * sin (th_half * 0.5) / sin (th_half);
    x1 = xc + cos (th0) - t * sin (th0);
    y1 = yc + sin (th0) + t * cos (th0);
    x3 = xc + cos (th1);
    y3 = yc + sin (th1);
    x2 = x3 + t * sin (th1);
    y2 = y3 - t * cos (th1);

    cairo_curve_to (cr, a00 * x1 + a01 * y1, a10 * x1 + a11 * y1,
		a00 * x2 + a01 * y2, a10 * x2 + a11 * y2,
		a00 * x3 + a01 * y3, a10 * x3 + a11 * y3);
}

/**
 * _svg_cairo_path_arc_to: Add an arc to the given path
 *
 * rx: Radius in x direction (before rotation).
 * ry: Radius in y direction (before rotation).
 * x_axis_rotation: Rotation angle for axes.
 * large_arc_flag: 0 for arc length <= 180, 1 for arc >= 180.
 * sweep: 0 for "negative angle", 1 for "positive angle".
 * x: New x coordinate.
 * y: New y coordinate.
 *
 **/
static svg_status_t
_svg_cairo_arc_to (void		*closure,
	      double		rx,
	      double		ry,
	      double		x_axis_rotation,
	      int		large_arc_flag,
	      int		sweep_flag,
	      double		x,
	      double		y)
{
    svg_cairo_t *svg_cairo = closure;
    double sin_th, cos_th;
    double a00, a01, a10, a11;
    double x0, y0, x1, y1, xc, yc;
    double d, sfactor, sfactor_sq;
    double th0, th1, th_arc;
    int i, n_segs;
    double dx, dy, dx1, dy1, Pr1, Pr2, Px, Py, check;
    double curx, cury;

    rx = fabs (rx);
    ry = fabs (ry);

    cairo_get_current_point (svg_cairo->cr, &curx, &cury);

    sin_th = sin (x_axis_rotation * (M_PI / 180.0));
    cos_th = cos (x_axis_rotation * (M_PI / 180.0));

    dx = (curx - x) / 2.0;
    dy = (cury - y) / 2.0;
    dx1 =  cos_th * dx + sin_th * dy;
    dy1 = -sin_th * dx + cos_th * dy;
    Pr1 = rx * rx;
    Pr2 = ry * ry;
    Px = dx1 * dx1;
    Py = dy1 * dy1;
    /* Spec : check if radii are large enough */
    check = Px / Pr1 + Py / Pr2;
    if(check > 1)
    {
        rx = rx * sqrt(check);
        ry = ry * sqrt(check);
    }

    a00 = cos_th / rx;
    a01 = sin_th / rx;
    a10 = -sin_th / ry;
    a11 = cos_th / ry;
    x0 = a00 * curx + a01 * cury;
    y0 = a10 * curx + a11 * cury;
    x1 = a00 * x + a01 * y;
    y1 = a10 * x + a11 * y;
    /* (x0, y0) is current point in transformed coordinate space.
       (x1, y1) is new point in transformed coordinate space.
       
       The arc fits a unit-radius circle in this space.
    */
    d = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
    sfactor_sq = 1.0 / d - 0.25;
    if (sfactor_sq < 0) sfactor_sq = 0;
    sfactor = sqrt (sfactor_sq);
    if (sweep_flag == large_arc_flag) sfactor = -sfactor;
    xc = 0.5 * (x0 + x1) - sfactor * (y1 - y0);
    yc = 0.5 * (y0 + y1) + sfactor * (x1 - x0);
    /* (xc, yc) is center of the circle. */
    
    th0 = atan2 (y0 - yc, x0 - xc);
    th1 = atan2 (y1 - yc, x1 - xc);
    
    th_arc = th1 - th0;
    if (th_arc < 0 && sweep_flag)
	th_arc += 2 * M_PI;
    else if (th_arc > 0 && !sweep_flag)
	th_arc -= 2 * M_PI;

    /* XXX: I still need to evaluate the math performed in this
       function. The critical behavior desired is that the arc must be
       approximated within an arbitrary error tolerance, (which the
       user should be able to specify as well). I don't yet know the
       bounds of the error from the following computation of
       n_segs. Plus the "+ 0.001" looks just plain fishy. -cworth */
    n_segs = ceil (fabs (th_arc / (M_PI * 0.5 + 0.001)));
    
    for (i = 0; i < n_segs; i++) {
	_svg_path_arc_segment (svg_cairo->cr, xc, yc,
					th0 + i * th_arc / n_segs,
					th0 + (i + 1) * th_arc / n_segs,
					rx, ry, x_axis_rotation);
    }

    return _cairo_status_to_svg_status (cairo_status (svg_cairo->cr));
}


