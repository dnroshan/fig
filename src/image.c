#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <jpeglib.h>
#include "image.h"
#include "misc.h"

static void read_jpeg_image(const char *file_path, FigImage *image);
static void write_jpeg_image(const char *file_path, FigImage *image);

FigImage *
fig_image_new(uint32_t width, uint32_t height)
{
    FigImage *image;
    unsigned char *data;

    image = malloc(sizeof *image);
    if (!image)
        fig_panic("failed allocating memeory");

    data = malloc(width * height * 3);
    if (!data)
        fig_panic("failed allocating memeory");

    image->width = width;
    image->height = height;
    image->data = data;

    return image;
}

FigImage *
fig_image_read(const char *file_path)
{
    FigImage *image;

    image = malloc(sizeof *image);
    if (!image)
        fig_panic("failed allocating memeory");

    read_jpeg_image(file_path, image);

    return image;
}

void
fig_image_write(FigImage *image, const char *file_path)
{
    write_jpeg_image(file_path, image);
}

static void 
read_jpeg_image(const char *file_path, FigImage *image)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    unsigned long int img_width, img_height;
    unsigned char *img_buffer;
    unsigned char *row_buffer[1];
    int n_components;
    FILE *fp;

    fp = fopen(file_path, "rb");
    if (!fp)
        fig_panic("unable to open file");

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, fp);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    img_width = cinfo.output_width;
    img_height = cinfo.output_height;
    n_components = cinfo.num_components;
    assert(n_components == 3);
    
    img_buffer = malloc(img_width * img_height * n_components);
    if (!img_buffer) {
        fclose(fp);
        fig_panic("failed allocating memeory");
    }

    while (cinfo.output_scanline < cinfo.output_height) {
        row_buffer[0] = img_buffer + cinfo.output_scanline * img_width * n_components;
        jpeg_read_scanlines(&cinfo, row_buffer, 1);
    }

    image->width = img_width;
    image->height = img_height;
    image->data = img_buffer;
    
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(fp);
}

static void 
write_jpeg_image(const char *file_path, FigImage *image)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    unsigned char *row_buffer[1];
    FILE *fp;

    fp = fopen(file_path, "wb");
    if (!fp)
        fig_panic("unable to open file");

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, fp);
    
    cinfo.image_width = image->width;
    cinfo.image_height = image->height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 100, TRUE);

    jpeg_start_compress(&cinfo, TRUE);
    
    while (cinfo.next_scanline < cinfo.image_height) {
        row_buffer[0] = image->data + cinfo.next_scanline * image->width * 3;
        jpeg_write_scanlines(&cinfo, row_buffer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(fp); 
}

FigImage *
fig_image_resize(FigImage *image, uint32_t width, uint32_t height)
{
    FigImage *output;
    float scale_x = width / (float) image->width;
    float scale_y = height / (float) image->height;
    float float_src_x, float_src_y, EW_weight, NS_weight;
    uint32_t src_x, src_y, src_offset, dest_offset;;
    float NW, NE, SW, SE, top, bottom, value;;
   
    output = fig_image_new(width, height);

    for (uint32_t dest_y = 0; dest_y < output->height; dest_y++) {
        for (uint32_t dest_x = 0; dest_x < output->width; dest_x++) {
            dest_offset = 3 * (dest_y * width + dest_x);

            float_src_x = dest_x / scale_x;
            float_src_y = dest_y / scale_y;

            src_x = (uint32_t) float_src_x;
            src_y = (uint32_t) float_src_y;

            EW_weight = float_src_x - src_x;
            NS_weight = float_src_y - src_y;

            src_offset = 3 * (src_y * image->width + src_x);

            for (uint32_t c = 0; c < 3; c++) {
                NW = (float) image->data[src_offset + c]; 
                NE = (float) image->data[src_offset + 3 + c]; 
                SW = (float) image->data[src_offset + width * 3 + c]; 
                SE = (float) image->data[src_offset + width * 3 + 3 + c]; 

                top = NW + EW_weight * (NE - NW);
                bottom = SW + EW_weight * (SE - SW);
                value = (unsigned char) (top + NS_weight * (bottom - top));
                value = value > 255 ? 255: value;
                output->data[dest_offset + c] = value;
            }
        }
    }

    return output;
}

void
fig_image_destroy(FigImage *image)
{
    if (image->data)
        free(image->data);
    free(image);
}
