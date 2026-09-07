#include "operations.h"

#include <stddef.h>
#include <stdlib.h>

static void swap_pixels(Image *image, int x1, int y1, int x2, int y2)
{
    unsigned char *pixel1 = get_pixel(image, x1, y1);
    unsigned char *pixel2 = get_pixel(image, x2, y2);

    if (pixel1 == NULL || pixel2 == NULL) {
        return;
    }

    for (int i = 0; i < image->channels; i++) {
        unsigned char temp = pixel1[i];
        pixel1[i] = pixel2[i];
        pixel2[i] = temp;
    }
}

static int clamp_value(int value, int min_value, int max_value)
{
    if (value < min_value) {
        return min_value;
    }

    if (value > max_value) {
        return max_value;
    }

    return value;
}

static void mirror_area_by_x(Image *image, int left, int top, int right, int bottom)
{
    for (int y = top; y < bottom; y++) {
        int x1 = left;
        int x2 = right - 1;

        while (x1 < x2) {
            swap_pixels(image, x1, y, x2, y);
            x1++;
            x2--;
        }
    }
}

static void mirror_area_by_y(Image *image, int left, int top, int right, int bottom)
{
    for (int x = left; x < right; x++) {
        int y1 = top;
        int y2 = bottom - 1;

        while (y1 < y2) {
            swap_pixels(image, x, y1, x, y2);
            y1++;
            y2--;
        }
    }
}

ErrorCode mirror_image(Image *image, char axis, Point left_up, Point right_down)
{
    if (image == NULL || image->data == NULL) {
        return ERROR_INVALID_ARGUMENTS;
    }

    if (axis != 'x' && axis != 'y') {
        return ERROR_INVALID_ARGUMENTS;
    }

    int left = clamp_value(left_up.x, 0, image->width);
    int right = clamp_value(right_down.x, 0, image->width);
    int top = clamp_value(left_up.y, 0, image->height);
    int bottom = clamp_value(right_down.y, 0, image->height);

    if (left >= right || top >= bottom) {
        return ERROR_SUCCESS;
    }

    if (axis == 'x') {
        mirror_area_by_x(image, left, top, right, bottom);
        return ERROR_SUCCESS;
    }

    mirror_area_by_y(image, left, top, right, bottom);

    return ERROR_SUCCESS;
}

static void copy_pixel_channels(unsigned char *destination, const unsigned char *source, int channels)
{
    for (int channel = 0; channel < channels; channel++) {
        destination[channel] = source[channel];
    }
}

ErrorCode copy_image(Image *image, Point left_up, Point right_down, Point dest_left_up)
{
    if (image == NULL || image->data == NULL) {
        return ERROR_INVALID_ARGUMENTS;
    }

    int src_x0 = left_up.x;
    int src_y0 = left_up.y;

    int dst_x0 = dest_left_up.x;
    int dst_y0 = dest_left_up.y;

    int copy_width = right_down.x - left_up.x;
    int copy_height = right_down.y - left_up.y;

    if (copy_width <= 0 || copy_height <= 0) {
        return ERROR_SUCCESS;
    }

    unsigned char *buffer = malloc(copy_width * copy_height * image->channels);
    if (!buffer) {
        return ERROR_MEMORY_ALLOCATION;
    }

    for (int dy = 0; dy < copy_height; dy++) {
        for (int dx = 0; dx < copy_width; dx++) {
            int source_x = src_x0 + dx;
            int source_y = src_y0 + dy;

            int dest_x = dst_x0 + dx;
            int dest_y = dst_y0 + dy;

            unsigned char *src_pixel = get_pixel(image, source_x, source_y);
            unsigned char *dest_pixel = get_pixel(image, dest_x, dest_y);
            if (src_pixel == NULL || dest_pixel == NULL) {
                continue;
            }

            unsigned char *buffer_pixel = buffer + (dy * copy_width + dx) * image->channels;

            copy_pixel_channels(buffer_pixel, src_pixel, image->channels);
        }
    }

    for (int dy = 0; dy < copy_height; dy++) {
        for (int dx = 0; dx < copy_width; dx++) {
            int source_x = src_x0 + dx;
            int source_y = src_y0 + dy;

            int dest_x = dst_x0 + dx;
            int dest_y = dst_y0 + dy;

            unsigned char *src_pixel = get_pixel(image, source_x, source_y);
            unsigned char *dest_pixel = get_pixel(image, dest_x, dest_y);
            if (src_pixel == NULL || dest_pixel == NULL) {
                continue;
            }

            unsigned char *buffer_pixel = buffer + (dy * copy_width + dx) * image->channels;

            copy_pixel_channels(dest_pixel, buffer_pixel, image->channels);
        }
    }

    free(buffer);
    return ERROR_SUCCESS;
}

static void set_color(Image *image, int x, int y, Color color)
{
    unsigned char *pixel = get_pixel(image, x, y);
    if (pixel == NULL) {
        return;
    }

    pixel[0] = color.r;
    pixel[1] = color.g;
    pixel[2] = color.b;
}

ErrorCode draw_circle(Image *image, Point center, int radius, int thickness, Color color, int fill,
                      Color fill_color)
{
    if (image == NULL || image->data == NULL || radius <= 0 || thickness <= 0) {
        return ERROR_INVALID_ARGUMENTS;
    }

    int inner_radius = radius - thickness / 2;
    int outer_radius = radius + (thickness + 1) / 2;

    if (inner_radius < 0) {
        inner_radius = 0;
    }

    int inner_radius_squared = inner_radius * inner_radius;
    int outer_radius_squared = outer_radius * outer_radius;

    if (fill == 1) {
        for (int x = center.x - inner_radius; x <= center.x + inner_radius; x++) {
            for (int y = center.y - inner_radius; y <= center.y + inner_radius; y++) {
                int dx = center.x - x;
                int dy = center.y - y;
                int distance = dx * dx + dy * dy;

                if (distance <= inner_radius_squared) {
                    set_color(image, x, y, fill_color);
                }
            }
        }
    }

    for (int x = center.x - outer_radius; x <= center.x + outer_radius; x++) {
        for (int y = center.y - outer_radius; y <= center.y + outer_radius; y++) {
            int dx = center.x - x;
            int dy = center.y - y;
            int distance = dx * dx + dy * dy;

            if (distance <= outer_radius_squared && (inner_radius == 0 || distance > inner_radius_squared)) {
                set_color(image, x, y, color);
            }
        }
    }

    return ERROR_SUCCESS;
}