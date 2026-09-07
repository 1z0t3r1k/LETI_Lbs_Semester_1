#include "image.h"

#include <stdio.h>
#include <stdlib.h>

int is_point_inside_image(const Image *image, Point point)
{
    if (image == NULL) {
        return 0;
    }

    return point.x >= 0 && point.x < image->width && point.y >= 0 && point.y < image->height;
}

unsigned char *get_pixel(Image *image, int x, int y)
{
    Point point = {x, y};
    if (image == NULL || image->data == NULL || !is_point_inside_image(image, point)) {
        return NULL;
    }

    return image->data + (y * image->width + x) * image->channels;
}

void free_image(Image *image)
{
    if (image == NULL) {
        return;
    }

    free(image->data);

    image->data = NULL;
    image->width = 0;
    image->height = 0;
    image->bit_depth = 0;
    image->color_type = 0;
    image->channels = 0;
}
