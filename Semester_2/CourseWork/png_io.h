#ifndef COURSEWORK_PNG_IO_H
#define COURSEWORK_PNG_IO_H

#include "errors.h"
#include "image.h"

ErrorCode read_png(const char *filename, Image *image);

ErrorCode write_png(const char *filename, const Image *image);

void print_png_info(const Image *image);

#endif // COURSEWORK_PNG_IO_H