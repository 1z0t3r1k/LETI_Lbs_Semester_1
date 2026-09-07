#include <stdio.h>

#include "cli.h"
#include "operations.h"
#include "png_io.h"

static ErrorCode run_info(const ProgramOptions *options)
{
    Image image = {0};
    ErrorCode error = read_png(options->input_file, &image);

    if (error != ERROR_SUCCESS) {
        fprintf(stderr, "Error: cannot read PNG file, code: %d\n", error);
    } else {
        print_png_info(&image);
        free_image(&image);
    }

    return error;
}

static ErrorCode run_mirror(const ProgramOptions *options)
{
    Image image = {0};
    ErrorCode error = read_png(options->input_file, &image);

    if (error != ERROR_SUCCESS) {
        fprintf(stderr, "Error: cannot read PNG file, code: %d\n", error);
    } else {
        error = mirror_image(&image, options->axis, options->left_up, options->right_down);
        if (error != ERROR_SUCCESS) {
            fprintf(stderr, "Error: cannot mirror PNG file, code: %d\n", error);
        }

        if (error == ERROR_SUCCESS) {
            error = write_png(options->output_file, &image);
            if (error != ERROR_SUCCESS) {
                fprintf(stderr, "Error: cannot write PNG file, code: %d\n", error);
            }
        }

        free_image(&image);
    }

    return error;
}

static ErrorCode run_copy(const ProgramOptions *options)
{
    Image image = {0};
    ErrorCode error = read_png(options->input_file, &image);

    if (error != ERROR_SUCCESS) {
        fprintf(stderr, "Error: cannot read PNG file, code: %d\n", error);
    } else {
        error = copy_image(&image, options->left_up, options->right_down, options->dest_left_up);
        if (error != ERROR_SUCCESS) {
            fprintf(stderr, "Error: cannot copy PNG file, code: %d\n", error);
        }

        if (error == ERROR_SUCCESS) {
            error = write_png(options->output_file, &image);
            if (error != ERROR_SUCCESS) {
                fprintf(stderr, "Error: cannot write PNG file, code: %d\n", error);
            }
        }

        free_image(&image);
    }

    return error;
}

static ErrorCode run_circle(const ProgramOptions *options)
{
    Image image = {0};
    ErrorCode error = read_png(options->input_file, &image);

    if (error != ERROR_SUCCESS) {
        fprintf(stderr, "Error: cannot read PNG file, code: %d\n", error);
    } else {
        error = draw_circle(&image, options->center, options->radius, options->thickness, options->color,
                            options->fill, options->fill_color);
        if (error != ERROR_SUCCESS) {
            fprintf(stderr, "Error: cannot draw circle, code: %d\n", error);
        }

        if (error == ERROR_SUCCESS) {
            error = write_png(options->output_file, &image);
            if (error != ERROR_SUCCESS) {
                fprintf(stderr, "Error: cannot write PNG file, code: %d\n", error);
            }
        }

        free_image(&image);
    }

    return error;
}

static ErrorCode run_action(const ProgramOptions *options, const char *program_name)
{
    switch (options->action) {
        case ACTION_HELP:
            print_help(program_name);
            return ERROR_SUCCESS;
        case ACTION_INFO:
            return run_info(options);
        case ACTION_MIRROR:
            return run_mirror(options);
        case ACTION_COPY:
            return run_copy(options);
        case ACTION_CIRCLE:
            return run_circle(options);
        default:
            fprintf(stderr, "Error: unsupported action\n");

            return ERROR_INVALID_ARGUMENTS;
    }
}

int main(int argc, char **argv)
{
    ProgramOptions options;
    init_options(&options);

    ErrorCode error = parse_arguments(argc, argv, &options);
    if (error != ERROR_SUCCESS) {
        fprintf(stderr, "Error: invalid command line arguments\n");
        return error;
    }

    return run_action(&options, argv[0]);
}