#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_image.h>
#include <unistd.h>
#include <stdlib.h>
static int screen_keys[512];	// 当前键盘按下状态
static unsigned char *screen_fb = NULL;		// frame buffer

static xcb_connection_t* c = NULL;
static xcb_gcontext_t gc, gctmp;
static xcb_window_t window;
static int window_w, window_h;
static int screen_exit = 0;
static xcb_image_t *image;
static xcb_screen_t *screen = NULL;

typedef char TCHAR;

#define _T(x) x
#define VK_ESCAPE 1
#define VK_UP 2
#define VK_DOWN 3
#define VK_LEFT 4 
#define VK_RIGHT 5
#define VK_SPACE 6


static int screen_init(int w, int h, const char *title) {
    window_w = w, window_h = h;

    c = xcb_connect(NULL, NULL);
    screen = xcb_setup_roots_iterator(xcb_get_setup(c)).data;

    window = xcb_generate_id(c); 
    gc = xcb_generate_id(c);
    gctmp = xcb_generate_id(c);

    uint32_t maskwin = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK | XCB_CW_DONT_PROPAGATE;
    uint32_t valwin[] = {
        screen->black_pixel,
        XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_EXPOSURE,
        XCB_EVENT_MASK_BUTTON_PRESS,
    };
    uint32_t maskgc = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
    uint32_t valgc[] = {
        screen->white_pixel,
        0
    };

    xcb_create_window (c,
            24,
            window,
            screen->root,
            100, 100,
            w, h,
            10,
            XCB_WINDOW_CLASS_INPUT_OUTPUT,
            screen->root_visual,
            maskwin, valwin);

    xcb_change_property(c, XCB_PROP_MODE_REPLACE, window,
            XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
            strlen(title), title);

    xcb_create_gc(c, gc, window, maskgc, valgc);
    xcb_create_gc(c, gctmp, window, 0, NULL);
    xcb_map_window(c, window);
    xcb_flush(c);

    image = xcb_image_create_native(c, 
            window_w, window_h,
            XCB_IMAGE_FORMAT_Z_PIXMAP,
            24,
            NULL,
            0,
            NULL);
    screen_fb = image->data;


    return 0;
}

static int screen_close(void) {
    xcb_disconnect(c);
    return 1;
}

static void screen_update(void) {
    xcb_void_cookie_t cookie = xcb_image_put(c,
            window,
            gc,
            image,
            0, 0,
            0
            );

    xcb_generic_error_t *err = xcb_request_check(c, cookie);
    if (err) {
        fprintf(stderr, "%d\n", err->error_code);   
    }
    xcb_flush(c);
}

static void screen_dispatch(void) {
    xcb_generic_event_t *event = xcb_poll_for_event(c);
    xcb_key_press_event_t *e;
    while (event) {
        switch (event->response_type & ~0x80) {
            case XCB_EXPOSE:
                screen_update();
                break;
            case XCB_KEY_PRESS:
                 e = (xcb_key_press_event_t*)event;
                 switch (e->detail) {
                     case 111:
                         screen_keys[VK_UP] = 1;
                         break; 
                     case 116:
                         screen_keys[VK_DOWN] = 1;
                         break; 
                     case 113:
                         screen_keys[VK_LEFT] = 1;
                         break; 
                     case 114:
                         screen_keys[VK_RIGHT] = 1;
                         break; 
                     case 9:
                         screen_keys[VK_ESCAPE] = 1;
                         break; 
                     case 65:
                         screen_keys[VK_SPACE] = 1;
                         break; 
                     default:
                         break; 
                 }
                break;
            case XCB_KEY_RELEASE:
                 e = (xcb_key_press_event_t*)event;
                 switch (e->detail) {
                     case 111:
                         screen_keys[VK_UP] = 0;
                         break; 
                     case 116:
                         screen_keys[VK_DOWN] = 0;
                         break; 
                     case 113:
                         screen_keys[VK_LEFT] = 0;
                         break; 
                     case 114:
                         screen_keys[VK_RIGHT] = 0;
                         break; 
                     case 9:
                         screen_keys[VK_ESCAPE] = 0;
                         break; 
                     case 65:
                         screen_keys[VK_SPACE] = 0;
                         break; 
                     default:
                         break; 
                 }
                break;
            default:
                break;
        }
        event = xcb_poll_for_event(c);
    }
}

static int Sleep(int n) 
{
    screen_dispatch();
    usleep(n *10);
}
