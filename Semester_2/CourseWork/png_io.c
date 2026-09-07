#include <stdio.h>

#include "png_io.h"

#include <png.h>
#include <stdlib.h>
#include <string.h>

void print_png_info(const Image *image)
{
    if (image == NULL) {
        return;
    }

    printf("Image width: %d\n", image->width);
    printf("Image height: %d\n", image->height);
    printf("Image bit depth: %d\n", image->bit_depth);
    printf("Image channels: %d\n", image->channels);
    printf("Image color type: %d\n", image->color_type);
}

static void png_read_callback(png_structp png, png_bytep data, png_size_t length)
{
    FILE *file = (FILE *)png_get_io_ptr(png);

    if (fread(data, 1, length, file) != length) {
        png_error(png, "Read error");
    }
}

static unsigned char signature[8];
static const unsigned char png_signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};

static ErrorCode check_png_signature(FILE *file)
{
    size_t read_bytes = fread(signature, sizeof(unsigned char), 8, file);
    if (read_bytes != 8) {
        return ERROR_NOT_PNG;
    }

    if (memcmp(signature, png_signature, 8) != 0) {
        return ERROR_NOT_PNG;
    }

    return ERROR_SUCCESS;
}

static ErrorCode validate_png_format(const Image *image, png_size_t row_bytes)
{
    if (image->bit_depth != 8) {
        return ERROR_UNSUPPORTED_FORMAT;
    }

    if (image->color_type != PNG_COLOR_TYPE_RGB && image->color_type != PNG_COLOR_TYPE_RGBA) {
        return ERROR_UNSUPPORTED_FORMAT;
    }

    if (row_bytes != (png_size_t)(image->width * image->channels)) {
        return ERROR_UNSUPPORTED_FORMAT;
    }

    return ERROR_SUCCESS;
}

ErrorCode read_png(const char *filename, Image *image)
{
    if (filename == NULL || image == NULL) {
        return ERROR_INVALID_ARGUMENTS;
    }

    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        return ERROR_FILE_OPEN;
    }

    ErrorCode error = check_png_signature(file);
    if (error != ERROR_SUCCESS) {
        fclose(file);
        return error;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png == NULL) {
        fclose(file);
        return ERROR_LIBPNG;
    }

    png_infop info = png_create_info_struct(png);
    if (info == NULL) {
        png_destroy_read_struct(&png, NULL, NULL);
        fclose(file);
        return ERROR_LIBPNG;
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(file);
        return ERROR_LIBPNG;
    }

    png_set_read_fn(png, file, png_read_callback);
    png_set_sig_bytes(png, 8);
    png_read_info(png, info);

    image->width = png_get_image_width(png, info);
    image->height = png_get_image_height(png, info);
    image->bit_depth = png_get_bit_depth(png, info);
    image->color_type = png_get_color_type(png, info);
    image->channels = png_get_channels(png, info);
    image->data = NULL;

    png_size_t row_bytes = png_get_rowbytes(png, info);

    error = validate_png_format(image, row_bytes);
    if (error != ERROR_SUCCESS) {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(file);
        return error;
    }

    image->data = malloc(row_bytes * image->height);
    if (image->data == NULL) {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(file);
        return ERROR_MEMORY_ALLOCATION;
    }

    png_bytep *rows = malloc(sizeof(*rows) * image->height);
    if (rows == NULL) {
        free(image->data);
        image->data = NULL;

        png_destroy_read_struct(&png, &info, NULL);
        fclose(file);
        return ERROR_MEMORY_ALLOCATION;
    }

    for (int y = 0; y < image->height; y++) {
        rows[y] = image->data + y * row_bytes;
    }

    png_read_image(png, rows);
    free(rows);

    png_destroy_read_struct(&png, &info, NULL);
    fclose(file);

    return ERROR_SUCCESS;
}

static void png_write_callback(png_structp png, png_bytep data, png_size_t length)
{
    FILE *file = png_get_io_ptr(png);

    if (fwrite(data, 1, length, file) != length) {
        png_error(png, "Write error");
    }
}

static void png_flush_callback(png_structp png)
{
    FILE *file = png_get_io_ptr(png);
    fflush(file);
}

ErrorCode write_png(const char *filename, const Image *image)
{
    if (filename == NULL || image == NULL || image->data == NULL) {
        return ERROR_INVALID_ARGUMENTS;
    }

    if (image->bit_depth != 8) {
        return ERROR_UNSUPPORTED_FORMAT;
    }

    if (image->color_type != PNG_COLOR_TYPE_RGB && image->color_type != PNG_COLOR_TYPE_RGBA) {
        return ERROR_UNSUPPORTED_FORMAT;
    }

    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        return ERROR_FILE_OPEN;
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png == NULL) {
        fclose(file);
        return ERROR_LIBPNG;
    }

    png_infop info = png_create_info_struct(png);
    if (info == NULL) {
        png_destroy_write_struct(&png, NULL);
        fclose(file);
        return ERROR_LIBPNG;
    }

    png_bytep *rows = NULL;

    if (setjmp(png_jmpbuf(png))) {
        free(rows);

        png_destroy_write_struct(&png, &info);
        fclose(file);
        return ERROR_LIBPNG;
    }

    png_set_write_fn(png, file, png_write_callback, png_flush_callback);

    png_set_compression_level(png, 0);

    png_set_IHDR(png, info, image->width, image->height, image->bit_depth, image->color_type,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png, info);

    png_size_t row_bytes = (png_size_t)(image->width * image->channels);

    rows = malloc(sizeof(png_bytep) * image->height);
    if (rows == NULL) {
        png_destroy_write_struct(&png, &info);
        fclose(file);
        return ERROR_MEMORY_ALLOCATION;
    }

    for (int y = 0; y < image->height; y++) {
        rows[y] = image->data + y * row_bytes;
    }

    png_write_image(png, rows);
    png_write_end(png, info);

    free(rows);

    png_destroy_write_struct(&png, &info);
    fclose(file);

    return ERROR_SUCCESS;
}