#ifndef COURSEWORK_CLI_H
#define COURSEWORK_CLI_H
#include "errors.h"
#include "image.h"

typedef enum { ACTION_NONE, ACTION_HELP, ACTION_INFO, ACTION_CIRCLE, ACTION_MIRROR, ACTION_COPY } ActionType;

typedef struct ProgramOptions {
    ActionType action;
    char *input_file;
    char *output_file;

    int has_axis;
    int has_left_up;
    int has_right_down;
    int has_dest_left_up;

    char axis;

    Point left_up;
    Point right_down;
    Point dest_left_up;

    Point center;
    Color color;
    Color fill_color;

    int radius;
    int thickness;
    int fill;

    int has_center;
    int has_radius;
    int has_thickness;
    int has_color;
    int has_fill_color;
} ProgramOptions;

void init_options(ProgramOptions *options);

ErrorCode parse_arguments(int argc, char **argv, ProgramOptions *options);

void print_help(const char *program_name);

#endif // COURSEWORK_CLI_H