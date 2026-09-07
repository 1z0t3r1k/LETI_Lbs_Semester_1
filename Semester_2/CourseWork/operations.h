#ifndef COURSEWORK_OPERATIONS_H
#define COURSEWORK_OPERATIONS_H
#include "errors.h"
#include "image.h"

ErrorCode mirror_image(Image *image, char axis, Point left_up, Point right_down);

ErrorCode copy_image(Image *image, Point left_up, Point right_down, Point dest_left_up);

ErrorCode draw_circle(Image *image, Point center, int radius, int thickness, Color color, int fill,
                      Color fill_color);

#endif // COURSEWORK_OPERATIONS_H