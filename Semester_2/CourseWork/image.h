#ifndef COURSEWORK_IMAGE_H
#define COURSEWORK_IMAGE_H

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} Color;

typedef struct {
    int width;
    int height;
    int bit_depth;
    int color_type;
    int channels;
    unsigned char *data;
} Image;

int is_point_inside_image(const Image *image, Point point);

unsigned char *get_pixel(Image *image, int x, int y);

void free_image(Image *image);

#endif // COURSEWORK_IMAGE_H