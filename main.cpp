#define GL_GLEXT_PROTOTYPES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>


#include "image.hpp"

int ctx_{0};

int viewport_width_{0};
int viewport_height_{0};

int pixel_width_{0};
int pixel_height_{0};

double pixel_ratio_{1};

float iter1{1};
float iter2{1};
float iter3{1};
float iter4{1};
float iter5{0};

float s{0.0};
float r1{0};
float r2{0};
float i1{0};
float i2{0};

float mx{0};
float my{0};

float pattern{1};

james::image* im{nullptr};

float projectionMatrix_[16];

void set_projection_matrix(int width, int height)
{
  projectionMatrix_[0] = 2.0f / width;
  projectionMatrix_[1] = 0;
  projectionMatrix_[2] = 0;
  projectionMatrix_[3] = 0;

  projectionMatrix_[4] = 0;
  projectionMatrix_[5] = 2.0f / height;
  projectionMatrix_[6] = 0;
  projectionMatrix_[7] = 0;

  projectionMatrix_[8] = 0;
  projectionMatrix_[9] = 0;
  projectionMatrix_[10] = -0.8/(float)std::max(width, height);
  projectionMatrix_[11] = -1.0/(float)std::max(width, height);

  projectionMatrix_[12] = -1;
  projectionMatrix_[13] = -1;
  projectionMatrix_[14] = 0.3; //0.6;  // 0.5
  projectionMatrix_[15] = 1;
}


const char * emscripten_event_type_to_string(int event_type) {
  const char *events[] = { "(invalid)", "(none)", "keypress", "keydown", "keyup", "click", "mousedown", "mouseup", "dblclick",
                           "mousemove", "wheel", "resize", "scroll", "blur", "focus", "focusin", "focusout", "deviceorientation",
                           "devicemotion", "orientationchange", "fullscreenchange", "pointerlockchange", "visibilitychange",
                           "touchstart", "touchend", "touchmove", "touchcancel", "gamepadconnected", "gamepaddisconnected",
                           "beforeunload", "batterychargingchange", "batterylevelchange", "webglcontextlost",
                           "webglcontextrestored", "mouseenter", "mouseleave", "mouseover", "mouseout", "(invalid)" };
  ++event_type;
  if (event_type < 0) event_type = 0;
  if (event_type >= sizeof(events)/sizeof(events[0])) event_type = sizeof(events)/sizeof(events[0])-1;

  return events[event_type];
};



EM_BOOL window_size_changed(int event_type, const EmscriptenUiEvent *uiEvent, void *user_data)
{

  if (event_type == EMSCRIPTEN_EVENT_RESIZE) {
    double w, h;
    EMSCRIPTEN_RESULT r = emscripten_get_element_css_size("canvas", &w, &h);
    if (r != EMSCRIPTEN_RESULT_SUCCESS) {
      fprintf(stderr, "Unable to call: emscripten_get_canvas_element_size\n");
      return false;
    }

    viewport_width_ = (int)w;
    viewport_height_ = (int)h;

    pixel_width_  = round(w * pixel_ratio_);
    pixel_height_ = round(h * pixel_ratio_);

    emscripten_set_canvas_element_size("canvas", pixel_width_, pixel_height_);

    glViewport(0, 0, pixel_width_, pixel_height_);
    set_projection_matrix(pixel_width_, pixel_height_);

    im->set_size(pixel_width_, pixel_height_);
    im->update(r1, r2, i1, i2);

    return EM_TRUE;
  }

  return EM_FALSE;
}


EM_BOOL mouse_callback(int event_type, const EmscriptenMouseEvent *e, void *user_data)
{
/*
  // for mousemove (could have multiple buttons pressed)

  if (e->buttons == 0) {
    incoming(source::MOUSE, 0, state::EMPTY, e->canvasX * gl::pixel_ratio_, (gl::viewport_height_ - e->canvasY) * gl::pixel_ratio_, false);
  } else {
    if (e->buttons & 1) incoming(source::BUTTON, 0, state::DOWN, e->canvasX * gl::pixel_ratio_, (gl::viewport_height_ - e->canvasY) * gl::pixel_ratio_, false);
    if (e->buttons & 2) incoming(source::BUTTON, 1, state::DOWN, e->canvasX * gl::pixel_ratio_, (gl::viewport_height_ - e->canvasY) * gl::pixel_ratio_, false);
    if (e->buttons & 4) incoming(source::BUTTON, 2, state::DOWN, e->canvasX * gl::pixel_ratio_, (gl::viewport_height_ - e->canvasY) * gl::pixel_ratio_, false);
  }

  // for others (individual button presses/releases fire off individual event callbacks)

  incoming(source::BUTTON, e->button, state::DOWN, e->canvasX * gl::pixel_ratio_, (gl::viewport_height_ - e->canvasY) * gl::pixel_ratio_, true);
*/

  switch (event_type) {
      case EMSCRIPTEN_EVENT_CLICK:      {
			if (s == 0) {
				r1 = -2.75;
				r2 = 1.5;
				i1 = -1.5;
				i2 = 1.25;

				iter1 = 1;
				iter2 = 1;
				iter3 = 0;
				iter4 = 0;
				iter5 = 0;
			}
				s = 0.0;

		  //r2 = (viewport_width_ - e->canvasX);
		  //r1 = -(viewport_width_ - e->canvasX) / viewport_width_ * 100000000;
        break;
      }
      case EMSCRIPTEN_EVENT_MOUSEDOWN:  {
			break;
      }
      case EMSCRIPTEN_EVENT_MOUSEUP:    {
        break;
      }
      case EMSCRIPTEN_EVENT_DBLCLICK:   {
		  //i1 = -(viewport_height_ - e->canvasY) * pixel_ratio_ / 100;
			break;
      }
		case EMSCRIPTEN_EVENT_MOUSEMOVE: {
			mx = ((float)viewport_width_ - (float)(e->canvasX)) / (float)viewport_width_;
			my = ((float)(e->canvasY)) / (float)viewport_height_;
			iter3 = r1 * mx + r2 * (1 - mx);
			iter4 = i1 * my + i2 * (1 - my);
			break;
		}
      //case EMSCRIPTEN_EVENT_MOUSEENTER: if (mouse_enter_cb_)    return mouse_enter_cb_(e); else break;
      //case EMSCRIPTEN_EVENT_MOUSELEAVE: if (mouse_leave_cb_)    return mouse_leave_cb_(e); else break;
      //case EMSCRIPTEN_EVENT_MOUSEOVER:  if (mouse_over_cb_)     return mouse_over_cb_(e); else break;
      //case EMSCRIPTEN_EVENT_MOUSEOUT:   if (mouse_out_cb_)      return mouse_out_cb_(e); else break;
      default: printf("Unknown event type: %d\n", event_type);
    }

  return EM_TRUE;
}

EM_BOOL touch_callback(int event_type, const EmscriptenTouchEvent *e, void *user_data)
{
/*
  for (const auto& t : e->touches) {
    if (t.isChanged) {
      incoming(source::TOUCH, t.identifier, state::DOWN, t.canvasX * gl::pixel_ratio_, (gl::viewport_height_ - t.canvasY) * gl::pixel_ratio_, true);
    }
  }
*/
  switch (event_type) {
      case EMSCRIPTEN_EVENT_TOUCHSTART:  {
        r1 *= 2;
        break;
      }
      case EMSCRIPTEN_EVENT_TOUCHEND:    {
        break;
      }
      case EMSCRIPTEN_EVENT_TOUCHMOVE:   {
        break;
      }
      case EMSCRIPTEN_EVENT_TOUCHCANCEL: {
        break;
      }
      default: printf("Unknown event type: %d\n", event_type);
    }

  return EM_TRUE;
}

EM_BOOL wheel_callback(int event_type, const EmscriptenWheelEvent *e, void *user_data)
{
	if (e->deltaY < 0) {
		if (s <= -0.0008) {
			s /= 1.25;
		} else if (s < 0) {
			s = 0;
		} else if (s != 0) {
			s *= 1.25;
		} else {
			s = 0.001;
		}
	} else {
		if (s >= 0.0008) {
			s /= 1.25;
		} else if (s > 0) {
			s = 0;
		} else if (s != 0) {
			s *= 1.25;
		} else {
			s = -0.001;
		}
	}

	return EM_TRUE;
}


EM_BOOL key_callback(int event_type, const EmscriptenKeyboardEvent *e, void *user_data)
{
//  printf("%s key: \"%s\", code: \"%s\", location: %lu,%s%s%s%s repeat: %d, locale: \"%s\", char: \"%s\", charCode: %lu, keyCode: %lu, which: %lu\n",
//             emscripten_event_type_to_string(event_type), e->key, e->code, e->location,
//             e->ctrlKey ? " CTRL" : "", e->shiftKey ? " SHIFT" : "", e->altKey ? " ALT" : "", e->metaKey ? " META" : "",
//             e->repeat, e->locale, e->charValue, e->charCode, e->keyCode, e->which);

  switch (event_type) {
      case EMSCRIPTEN_EVENT_KEYPRESS: {
        if (*(e->key) == 'w') {
          i1 += 0.2;
          i2 += 0.2;
        }
        break;
      }
      case EMSCRIPTEN_EVENT_KEYDOWN:  {
        if (*(e->key) == 'w') {
			 auto ritemp = (i1 + i2) / 2;
			 i1 -= 0.18*(i1 - ritemp);
			 i2 += 0.18*(i2 - ritemp);
          //i1 += 0.2;
          //i2 += 0.2;
		  } else if (*(e->key) == 's') {
			  auto ritemp = (i1 + i2) / 2;
			  i1 += 0.18*(i1 - ritemp);
			  i2 -= 0.18*(i2 - ritemp);
		  } else if (*(e->key) == 'a') {
			  auto ritemp = (r1 + r2) / 2;
			  r1 += 0.18*(r1 - ritemp);
			  r2 -= 0.18*(r2 - ritemp);
		  } else if (*(e->key) == 'd') {
			  auto ritemp = (r1 + r2) / 2;
			  r1 -= 0.18*(r1 - ritemp);
			  r2 += 0.18*(r2 - ritemp);
		  } else if (*(e->key) == 'm') {
			  iter5 = 1;
		  } else if (*(e->key) == 'f') {
			  if (s <= -0.0008) {
				  s /= 1.5;
			  } else if (s < 0) {
				  s = 0;
			  } else if (s != 0) {
				  s *= 1.5;
			  } else {
				  s = 0.001;
			  }
		  } else if (*(e->key) == 'g') {
			  if (s >= 0.0008) {
				  s /= 1.5;
			  } else if (s > 0) {
				  s = 0;
			  } else if (s != 0) {
				  s *= 1.5;
			  } else {
				  s = -0.001;
			  }
		  } else if (*(e->key) == 'p') {
				pattern = 1;
			} else if (*(e->key) == 'o') {
				pattern = 0;
			}
        break;
      }
      case EMSCRIPTEN_EVENT_KEYUP:    {
        break;
      }
      default: printf("Unknown event type: %d\n", event_type);
    }

  return EM_TRUE;
}


void generate_frame() {
  glClear(GL_COLOR_BUFFER_BIT);

  // james's draw
  im->display(0, 0, iter1++, iter2++, iter3, iter4);// , iter5);
	/*
	if (iter1 == 200) {
		iter2++;
		iter1 = 0;
	}*/
  /*
  r1 *= (1-s);
  r2 *= (1 - s);
  i1 *= (1 - s);
  i2 *= (1 - s);*/

  //ritemp = r1 + (-r1 * (1 - mx) + r2 * mx) / 2;

  auto ritemp = r1 * mx + r2 * (1 - mx);

  r1 -= s * (r1 - ritemp);
  r2 -= s * (r2 - ritemp);

  ritemp = i1 * my + i2 * (1 - my);
  i1 -= s * (i1 - ritemp);
  i2 -= s * (i2 - ritemp);

  im->update(r1, r2, i1, i2);
}
 
int main(int argc, char *argv[])
{

  double w, h;
  EMSCRIPTEN_RESULT r = emscripten_get_element_css_size("canvas", &w, &h);

  viewport_width_ = (int)w;
  viewport_height_ = (int)h;

  pixel_ratio_ = emscripten_get_device_pixel_ratio();

  pixel_width_  = round(w * pixel_ratio_);
  pixel_height_ = round(h * pixel_ratio_);

  // window ui events
  auto res = emscripten_set_resize_callback(0, nullptr, 1, window_size_changed);

  // mouse events
  res = emscripten_set_click_callback(0, nullptr, 1, mouse_callback);
  res = emscripten_set_mousedown_callback(0, nullptr, 1, mouse_callback);
  res = emscripten_set_mouseup_callback(0, nullptr, 1, mouse_callback);
  res = emscripten_set_dblclick_callback(0, nullptr, 1, mouse_callback);
  res = emscripten_set_mousemove_callback(0, nullptr, 1, mouse_callback);

  // touch events
  res = emscripten_set_touchstart_callback(0, nullptr, 1, touch_callback);
  res = emscripten_set_touchend_callback(0, nullptr, 1, touch_callback);
  res = emscripten_set_touchmove_callback(0, nullptr, 1, touch_callback);
  res = emscripten_set_touchcancel_callback(0, nullptr, 1, touch_callback);

  // key events
  res = emscripten_set_keypress_callback(0, nullptr, 1, key_callback);
  res = emscripten_set_keydown_callback(0, nullptr, 1, key_callback);
  res = emscripten_set_keyup_callback(0, nullptr, 1, key_callback);

  // wheel events
  res = emscripten_set_wheel_callback(0, nullptr, 1, wheel_callback);

  emscripten_set_canvas_element_size("canvas", pixel_width_, pixel_height_);

  EmscriptenWebGLContextAttributes ctxAttrs;
  emscripten_webgl_init_context_attributes(&ctxAttrs);
  ctxAttrs.antialias = true;
  ctx_ = emscripten_webgl_create_context(nullptr, &ctxAttrs);
  emscripten_webgl_make_context_current(ctx_);

  // init james's shaders

  set_projection_matrix(pixel_width_, pixel_height_);
  glViewport(0, 0, pixel_width_, pixel_height_);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glClearColor(1.0, 1.0, 1.0, 1.0);

  // create texture space for 400x400 image with 4 bytes of depth for RGBA
//  std::uint8_t * data = (std::uint8_t *)malloc(pixel_width_ * pixel_height_ * 4);
//  memset(data, 0, pixel_width_ * pixel_height_ * 4);

  // create an image object (pushed data to the gpu)
  im = new james::image();

  im->load(nullptr, pixel_width_, pixel_height_);

  r1 = -2.75;
  r2 = 1.5;
  i1 = -1.5;
  i2 = 1.25;

  iter1 = 1;
  iter2 = 1;
  iter3 = 0;
  iter4 = 0;
  iter5 = 0;

  emscripten_set_main_loop(generate_frame, 0, 0);
}

