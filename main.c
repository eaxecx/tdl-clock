#include <u8string.h>
#include <tdl.h>
#include <tdl/tdl_char.h>
#include <tdl/tdl_image.h>
#include <tdl/tdl_style.h>

/* On some platforms, the M_PI macro requires this macro to be defined. */
#define _USE_MATH_DEFINES
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* If the _USE_MATH_DEFINES did not help... */
#ifndef M_PI
# define M_PI 3.14159265358979323846
#endif

#define CLOCK_FACE_RADIUS  20
#define CLOCK_FACE_X_RATIO 2
#define SEC_HAND_LEN  18
#define MIN_HAND_LEN  15
#define HOUR_HAND_LEN 18
#define HAND_X_RATIO  1.8

#define MIN_MARK_CHAR   tdl_char("*", tdl_style(tdl_point_color(TDL_DEFAULT_COLOR, TDL_BLUE), TDL_NO_ATTRIBUTES))
#define HOUR_MARK_STYLE tdl_style(tdl_point_color(TDL_DEFAULT_COLOR, TDL_BRIGHT_GREEN), TDL_BOLD)

#define SEC_HAND_CHAR  tdl_char("s", tdl_style(tdl_point_color(TDL_DEFAULT_COLOR, TDL_BRIGHT_RED),   TDL_NO_ATTRIBUTES))
#define MIN_HAND_CHAR  tdl_char("m", tdl_style(tdl_point_color(TDL_DEFAULT_COLOR, TDL_BRIGHT_BLUE),  TDL_NO_ATTRIBUTES))
#define HOUR_HAND_CHAR tdl_char("h", tdl_style(tdl_point_color(TDL_DEFAULT_COLOR, TDL_BRIGHT_GREEN), TDL_NO_ATTRIBUTES))

#define CLOCK_MAX_WIDTH  CLOCK_FACE_RADIUS * 2 * CLOCK_FACE_X_RATIO + 4
#define CLOCK_MAX_HEIGHT CLOCK_FACE_RADIUS * 2 + 1

static tdl_canvas_t *_main_canvas = NULL;
static tdl_canvas_t *_clock_face_canvas = NULL;
static tdl_image_t _clock_face;

static void _draw_clock_face(tdl_canvas_t *canvas, tdl_point_t coords, int radius) {
    static const char *numbers[] = {
        "1", "2", "3", "4", "5", "6",
        "7", "8", "9", "10", "11", "12"
    };

    for (int i = 0; i < 60; ++i) {
        int x = coords.x + (int) (cos((i + 46) * 6 * M_PI / 180) * radius * CLOCK_FACE_X_RATIO);
        int y = coords.y + (int) (sin((i + 46) * 6 * M_PI / 180) * radius);

        if ((i + 1) % 5) {
            tdl_set_cursor_pos(canvas, tdl_point(x, y));

            tdl_putchar(canvas, MIN_MARK_CHAR);
        }
    }

    for (int i = 0; i < 12; ++i) {
        int x = coords.x + (int) (cos((i + 46) * 30 * M_PI / 180) * radius * CLOCK_FACE_X_RATIO);
        int y = coords.y + (int) (sin((i + 46) * 30 * M_PI / 180) * radius);

        tdl_set_cursor_pos(canvas, tdl_point(x, y));

        tdl_print(canvas, tdl_text(u8string((const cstr) numbers[i]), HOUR_MARK_STYLE));
    }
}

static void _draw_sec_hand(tdl_canvas_t *canvas, tdl_point_t coords, int secs) {
    int x = coords.x + (int) (cos((secs + 45) * 6 * M_PI / 180) * SEC_HAND_LEN * HAND_X_RATIO);
    int y = coords.y + (int) (sin((secs + 45) * 6 * M_PI / 180) * SEC_HAND_LEN);

    tdl_draw_line(canvas, SEC_HAND_CHAR, tdl_line(coords, tdl_point(x, y)));
}

static void _draw_min_hand(tdl_canvas_t *canvas, tdl_point_t coords, int min) {
    int x = coords.x + (int) (cos((min + 45) * 6 * M_PI / 180) * MIN_HAND_LEN * HAND_X_RATIO);
    int y = coords.y + (int) (sin((min + 45) * 6 * M_PI / 180) * MIN_HAND_LEN);

    tdl_draw_line(canvas, MIN_HAND_CHAR, tdl_line(coords, tdl_point(x, y)));
}

static void _draw_hour_hand(tdl_canvas_t *canvas, tdl_point_t coords, int mins, int hours) {
    int x = coords.x + (int) (cos(((hours + mins / 60.0) + 45) * 30 * M_PI / 180) * HOUR_HAND_LEN * HAND_X_RATIO);
    int y = coords.y + (int) (sin(((hours + mins / 60.0) + 45) * 30 * M_PI / 180) * HOUR_HAND_LEN);

    tdl_draw_line(canvas, HOUR_HAND_CHAR, tdl_line(coords, tdl_point(x, y)));
}

static void _deinit(void) {
    tdl_destroy_canvas(_clock_face_canvas);
    _clock_face_canvas = NULL;

    tdl_destroy_canvas(_main_canvas);
    _main_canvas = NULL;

    tdl_image_free(&_clock_face);

    tdl_terminal_set_alternate_screen(false);
    tdl_terminal_set_cursor(true);
}

static bool _init_graphics(void) {
    tdl_terminal_set_alternate_screen(true);
    tdl_terminal_set_cursor(false);

    memset(&_clock_face, 0, sizeof(tdl_image_t));

    _main_canvas = tdl_canvas();
    _clock_face_canvas = tdl_canvas();

    if (!(_main_canvas && _clock_face_canvas)) {
        _deinit();

        fprintf(stderr, "Cannot allocate a canvas. Possibly out of memory.\n");


        return false;
    }

    return true;
}

static void _setup_clock_face(tdl_point_t coords) {
    _draw_clock_face(_clock_face_canvas, coords, CLOCK_FACE_RADIUS);
    
    _clock_face = tdl_image_crop_from_canvas(_clock_face_canvas, tdl_rectangle(tdl_point(0, 0), _clock_face_canvas->size));

    tdl_destroy_canvas(_clock_face_canvas);
    _clock_face_canvas = NULL;
}

static void _interrupt_handler(int signal) {
    (void) signal;

    _deinit();

    exit(EXIT_SUCCESS);
}

static void _main_loop(tdl_point_t coords) {
    while (true) {
        time_t t = time(NULL);
        struct tm *local_time = localtime(&t);

        if (!local_time) {
            _deinit();

            fprintf(stderr, "Cannot retrieve current time via localtime().\n");

            exit(EXIT_FAILURE);
        }

        tdl_image_print_to_canvas(_main_canvas, _clock_face, tdl_point(0, 0));

        _draw_sec_hand(_main_canvas, coords, local_time->tm_sec);
        _draw_min_hand(_main_canvas, coords, local_time->tm_min);
        _draw_hour_hand(_main_canvas, coords, local_time->tm_min, local_time->tm_hour);

        tdl_display(_main_canvas);
    }
}

int main(void) {
    signal(SIGINT, _interrupt_handler);

    if (!_init_graphics()) {
        return EXIT_FAILURE;
    }

    if (_main_canvas->size.width < CLOCK_MAX_WIDTH || _main_canvas->size.height < CLOCK_MAX_HEIGHT) {
        _deinit();

        fprintf(stderr, "The terminal window is too small. The minimum size is %d columns and %d lines.\n", CLOCK_MAX_WIDTH, CLOCK_MAX_HEIGHT);

        return EXIT_FAILURE;
    }

    tdl_point_t clock_coords = tdl_point((int) _clock_face_canvas->size.width / 2, (int) _clock_face_canvas->size.height / 2);
    _setup_clock_face(clock_coords);

    _main_loop(clock_coords);

    return EXIT_SUCCESS;
}
