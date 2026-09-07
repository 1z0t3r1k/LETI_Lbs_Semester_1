#include "cli.h"

#include <getopt.h>
#include <stddef.h>
#include <stdio.h>

enum {
    OPT_MIRROR = 1000,
    OPT_AXIS,
    OPT_LEFT_UP,
    OPT_RIGHT_DOWN,
    OPT_COPY,
    OPT_DEST_LEFT_UP,
    OPT_CIRCLE,
    OPT_CENTER,
    OPT_RADIUS,
    OPT_THICKNESS,
    OPT_COLOR,
    OPT_FILL,
    OPT_FILL_COLOR
};

static const struct option LONG_OPTIONS[] = {{"help", no_argument, NULL, 'h'},
                                             {"info", no_argument, NULL, 'I'},
                                             {"output", required_argument, NULL, 'o'},
                                             {"input", required_argument, NULL, 'i'},

                                             {"mirror", no_argument, NULL, OPT_MIRROR},
                                             {"axis", required_argument, NULL, OPT_AXIS},
                                             {"left_up", required_argument, NULL, OPT_LEFT_UP},
                                             {"right_down", required_argument, NULL, OPT_RIGHT_DOWN},

                                             {"copy", no_argument, NULL, OPT_COPY},
                                             {"dest_left_up", required_argument, NULL, OPT_DEST_LEFT_UP},

                                             {"circle", no_argument, NULL, OPT_CIRCLE},
                                             {"center", required_argument, NULL, OPT_CENTER},
                                             {"radius", required_argument, NULL, OPT_RADIUS},
                                             {"thickness", required_argument, NULL, OPT_THICKNESS},
                                             {"color", required_argument, NULL, OPT_COLOR},
                                             {"fill", no_argument, NULL, OPT_FILL},
                                             {"fill_color", required_argument, NULL, OPT_FILL_COLOR},

                                             {0, 0, 0, 0}};

void init_options(ProgramOptions *options)
{
    if (options == NULL) {
        return;
    }

    options->action = ACTION_NONE;
    options->input_file = NULL;
    options->output_file = "out.png";

    options->axis = 0;

    options->left_up.x = 0;
    options->left_up.y = 0;
    options->right_down.x = 0;
    options->right_down.y = 0;
    options->dest_left_up.x = 0;
    options->dest_left_up.y = 0;

    options->center.x = 0;
    options->center.y = 0;

    options->radius = 0;
    options->thickness = 0;

    options->color.r = 0;
    options->color.g = 0;
    options->color.b = 0;

    options->fill = 0;

    options->fill_color.r = 0;
    options->fill_color.g = 0;
    options->fill_color.b = 0;

    options->has_axis = 0;
    options->has_left_up = 0;
    options->has_right_down = 0;
    options->has_dest_left_up = 0;

    options->has_center = 0;
    options->has_radius = 0;
    options->has_thickness = 0;
    options->has_color = 0;
    options->has_fill_color = 0;
}

static ErrorCode parse_point(const char *text, Point *point)
{
    if (point == NULL) {
        return ERROR_INVALID_ARGUMENTS;
    }

    int x;
    int y;
    char extra;

    if (sscanf(text, "%d.%d%c", &x, &y, &extra) != 2) {
        return ERROR_INVALID_ARGUMENTS;
    }

    point->x = x;
    point->y = y;

    return ERROR_SUCCESS;
}

static ErrorCode parse_color(const char *text, Color *color)
{
    if (color == NULL) {
        return ERROR_INVALID_ARGUMENTS;
    }

    int r;
    int g;
    int b;
    char extra;

    if (sscanf(text, "%d.%d.%d%c", &r, &g, &b, &extra) != 3) {
        return ERROR_INVALID_ARGUMENTS;
    }

    if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
        return ERROR_INVALID_ARGUMENTS;
    }

    color->r = r;
    color->g = g;
    color->b = b;

    return ERROR_SUCCESS;
}

static ErrorCode parse_positive_int(const char *text, int *value)
{
    if (value == NULL) {
        return ERROR_INVALID_ARGUMENTS;
    }

    int number;
    char extra;

    if (sscanf(text, "%d%c", &number, &extra) != 1) {
        return ERROR_INVALID_ARGUMENTS;
    }

    if (number <= 0) {
        return ERROR_INVALID_ARGUMENTS;
    }

    *value = number;

    return ERROR_SUCCESS;
}

static ErrorCode set_action(ProgramOptions *options, ActionType action)
{
    if (options->action != ACTION_NONE) {
        return ERROR_MULTIPLE_ACTIONS;
    }

    options->action = action;

    return ERROR_SUCCESS;
}

static ErrorCode handle_general_option(int option, const char *argument, ProgramOptions *options,
                                       int *handled)
{
    *handled = 1;

    switch (option) {
        case 'h':
            return set_action(options, ACTION_HELP);
        case 'I':
            return set_action(options, ACTION_INFO);
        case 'o':
            if (argument == NULL) {
                return ERROR_INVALID_ARGUMENTS;
            }

            options->output_file = (char *)argument;
            return ERROR_SUCCESS;
        case 'i':
            if (argument == NULL || options->input_file != NULL) {
                return ERROR_INVALID_ARGUMENTS;
            }

            options->input_file = (char *)argument;
            return ERROR_SUCCESS;
        case OPT_MIRROR:
            return set_action(options, ACTION_MIRROR);
        case OPT_COPY:
            return set_action(options, ACTION_COPY);
        case OPT_CIRCLE:
            return set_action(options, ACTION_CIRCLE);
        default:
            *handled = 0;
            return ERROR_SUCCESS;
    }
}

static ErrorCode parse_axis_option(const char *argument, ProgramOptions *options)
{
    if (argument == NULL || options->has_axis) {
        return ERROR_INVALID_ARGUMENTS;
    }

    if ((argument[0] != 'x' && argument[0] != 'y') || argument[1] != '\0') {
        return ERROR_INVALID_ARGUMENTS;
    }

    options->axis = argument[0];
    options->has_axis = 1;

    return ERROR_SUCCESS;
}

static ErrorCode parse_point_option(const char *argument, Point *point, int *has_point)
{
    if (argument == NULL || *has_point) {
        return ERROR_INVALID_ARGUMENTS;
    }

    ErrorCode code = parse_point(argument, point);
    if (code != ERROR_SUCCESS) {
        return code;
    }

    *has_point = 1;

    return ERROR_SUCCESS;
}

static ErrorCode handle_area_option(int option, const char *argument, ProgramOptions *options, int *handled)
{
    *handled = 1;

    switch (option) {
        case OPT_AXIS:
            return parse_axis_option(argument, options);
        case OPT_LEFT_UP:
            return parse_point_option(argument, &options->left_up, &options->has_left_up);
        case OPT_RIGHT_DOWN:
            return parse_point_option(argument, &options->right_down, &options->has_right_down);
        case OPT_DEST_LEFT_UP:
            return parse_point_option(argument, &options->dest_left_up, &options->has_dest_left_up);
        default:
            *handled = 0;
            return ERROR_SUCCESS;
    }
}

static ErrorCode parse_int_option(const char *argument, int *value, int *has_value)
{
    if (argument == NULL || *has_value) {
        return ERROR_INVALID_ARGUMENTS;
    }

    ErrorCode code = parse_positive_int(argument, value);
    if (code != ERROR_SUCCESS) {
        return code;
    }

    *has_value = 1;

    return ERROR_SUCCESS;
}

static ErrorCode parse_color_option(const char *argument, Color *color, int *has_color)
{
    if (argument == NULL || *has_color) {
        return ERROR_INVALID_ARGUMENTS;
    }

    ErrorCode code = parse_color(argument, color);
    if (code != ERROR_SUCCESS) {
        return code;
    }

    *has_color = 1;

    return ERROR_SUCCESS;
}

static ErrorCode handle_circle_option(int option, const char *argument, ProgramOptions *options, int *handled)
{
    *handled = 1;

    switch (option) {
        case OPT_CENTER:
            return parse_point_option(argument, &options->center, &options->has_center);
        case OPT_RADIUS:
            return parse_int_option(argument, &options->radius, &options->has_radius);
        case OPT_THICKNESS:
            return parse_int_option(argument, &options->thickness, &options->has_thickness);
        case OPT_COLOR:
            return parse_color_option(argument, &options->color, &options->has_color);
        case OPT_FILL:
            if (options->fill) {
                return ERROR_INVALID_ARGUMENTS;
            }

            options->fill = 1;
            return ERROR_SUCCESS;
        case OPT_FILL_COLOR:
            return parse_color_option(argument, &options->fill_color, &options->has_fill_color);
        default:
            *handled = 0;
            return ERROR_SUCCESS;
    }
}

static ErrorCode handle_option(int option, const char *argument, ProgramOptions *options)
{
    int handled = 0;

    ErrorCode code = handle_general_option(option, argument, options, &handled);

    if (code == ERROR_SUCCESS && !handled) {
        code = handle_area_option(option, argument, options, &handled);
    }

    if (code == ERROR_SUCCESS && !handled) {
        code = handle_circle_option(option, argument, options, &handled);
    }

    if (code == ERROR_SUCCESS && !handled) {
        code = ERROR_INVALID_ARGUMENTS;
    }

    return code;
}

static int has_area_options(const ProgramOptions *options)
{
    return options->has_axis || options->has_left_up || options->has_right_down || options->has_dest_left_up;
}

static int has_circle_options(const ProgramOptions *options)
{
    return options->has_center || options->has_radius || options->has_thickness || options->has_color ||
           options->fill || options->has_fill_color;
}

static ErrorCode validate_area_order(const ProgramOptions *options)
{
    if (options->left_up.x > options->right_down.x || options->left_up.y > options->right_down.y) {
        return ERROR_INVALID_ARGUMENTS;
    }

    return ERROR_SUCCESS;
}

static ErrorCode validate_info_options(const ProgramOptions *options)
{
    if (has_area_options(options) || has_circle_options(options)) {
        return ERROR_INVALID_ARGUMENTS;
    }

    return ERROR_SUCCESS;
}

static ErrorCode validate_mirror_options(const ProgramOptions *options)
{
    if (!options->has_axis || !options->has_left_up || !options->has_right_down ||
        options->has_dest_left_up || has_circle_options(options)) {
        return ERROR_INVALID_ARGUMENTS;
    }

    return validate_area_order(options);
}

static ErrorCode validate_copy_options(const ProgramOptions *options)
{
    if (!options->has_left_up || !options->has_right_down || !options->has_dest_left_up ||
        options->has_axis || has_circle_options(options)) {
        return ERROR_INVALID_ARGUMENTS;
    }

    return validate_area_order(options);
}

static ErrorCode validate_circle_options(const ProgramOptions *options)
{
    if (!options->has_center || !options->has_radius || !options->has_thickness || !options->has_color ||
        has_area_options(options)) {
        return ERROR_INVALID_ARGUMENTS;
    }

    if (options->fill && !options->has_fill_color) {
        return ERROR_INVALID_ARGUMENTS;
    }

    return ERROR_SUCCESS;
}

static ErrorCode validate_options(const ProgramOptions *options)
{
    switch (options->action) {
        case ACTION_HELP:
            return ERROR_SUCCESS;
        case ACTION_INFO:
            return validate_info_options(options);
        case ACTION_MIRROR:
            return validate_mirror_options(options);
        case ACTION_COPY:
            return validate_copy_options(options);
        case ACTION_CIRCLE:
            return validate_circle_options(options);
        default:
            return ERROR_INVALID_ARGUMENTS;
    }
}

static ErrorCode set_input_file(int argc, char **argv, ProgramOptions *options)
{
    if (options->input_file != NULL) {
        if (optind != argc) {
            return ERROR_INVALID_ARGUMENTS;
        }

        return ERROR_SUCCESS;
    }

    if (optind + 1 != argc) {
        return ERROR_INVALID_ARGUMENTS;
    }

    options->input_file = argv[optind];

    return ERROR_SUCCESS;
}

ErrorCode parse_arguments(int argc, char **argv, ProgramOptions *options)
{
    if (options == NULL) {
        return ERROR_INVALID_ARGUMENTS;
    }

    if (argc == 1) {
        options->action = ACTION_HELP;
        return ERROR_SUCCESS;
    }

    int option;

    while ((option = getopt_long(argc, argv, "hIo:i:", LONG_OPTIONS, NULL)) != -1) {
        ErrorCode code = handle_option(option, optarg, options);
        if (code != ERROR_SUCCESS) {
            return code;
        }
    }

    ErrorCode code = validate_options(options);
    if (code != ERROR_SUCCESS) {
        return code;
    }

    if (options->action == ACTION_HELP) {
        return ERROR_SUCCESS;
    }

    return set_input_file(argc, argv, options);
}

void print_help(const char *program_name)
{
    if (program_name == NULL) {
        program_name = "cw";
    }

    printf("Course work for option 4.13, created by Dmitrii Serpyakov\n\n");

    printf("Usage:\n");
    printf("  %s [OPTIONS] input.png\n\n", program_name);

    printf("General options:\n");
    printf("  -h, --help                 Show help\n");
    printf("  -i, --input FILE           Input PNG file\n");
    printf("  -o, --output FILE          Output PNG file, default: out.png\n");
    printf("      --info                 Print PNG information\n\n");

    printf("Circle:\n");
    printf("      --circle               Draw circle\n");
    printf("      --center X.Y           Circle center\n");
    printf("      --radius N             Circle radius, N > 0\n");
    printf("      --thickness N          Circle border thickness, N > 0\n");
    printf("      --color R.G.B          Circle border color\n");
    printf("      --fill                 Fill circle\n");
    printf("      --fill_color R.G.B     Fill color, used only with --fill\n\n");

    printf("Mirror:\n");
    printf("      --mirror               Mirror selected area\n");
    printf("      --axis x|y             Mirror axis\n");
    printf("      --left_up X.Y          Left upper corner of area\n");
    printf("      --right_down X.Y       Right lower corner of area, not included\n\n");

    printf("Copy:\n");
    printf("      --copy                 Copy selected area\n");
    printf("      --left_up X.Y          Left upper corner of source area\n");
    printf("      --right_down X.Y       Right lower corner of source area, not included\n");
    printf("      --dest_left_up X.Y     Left upper corner of destination area\n\n");

    printf("Examples:\n");
    printf("  %s --info input.png\n", program_name);
    printf("  %s --circle --center 300.300 --radius 100 --thickness 5 --color 255.0.0 input.png\n",
           program_name);
    printf("  %s --mirror --axis x --left_up 100.100 --right_down 500.400 input.png\n", program_name);
    printf("  %s --copy --left_up 100.100 --right_down 300.300 --dest_left_up 500.100 input.png\n",
           program_name);
}