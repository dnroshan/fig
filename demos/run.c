#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <image.h>
#include <buffer.h>
#include <model.h>

#define _ISOC99_SOURCE

struct Detection
{
    float xc, yc;
    float height, width;
    float conf;
};


void
preprocess(FigImage *image, FigBuffer *buffer)
{
    int offset;

    for (uint32_t y = 0; y < image->height; y++) {
        for (uint32_t x = 0; x < image->width; x++) {
            offset = fig_buffer_offset_of(buffer, x, y, 0);
            buffer->data[offset + 0] = image->data[offset + 2] / 255.0;
            buffer->data[offset + 1] = image->data[offset + 1] / 255.0;
            buffer->data[offset + 2] = image->data[offset + 0] / 255.0;
        }
    }
}

int
compare(const void *p, const void *q)
{
    int res;

    struct Detection *a = (struct Detection *) p;
    struct Detection *b = (struct Detection *) q;

    if (a->conf < b->conf)
        res = -1;
    else if (a->conf > b->conf)
        res = 1;
    else
        res = 0;

    return -1 * res;
}

int
main(int argc, char *argv[])
{
    FigImage *image, *image_resized;
    FigBuffer *input, *output;
    FigModel *model;
    struct Detection det[20 * 20];
    int i = 0;

    if (argc < 2)
        exit(1);

    input = fig_buffer_new(320, 320, 3);
    model = fig_model_from_file(argv[1], input);

    image = fig_image_read(argv[2]);
    image_resized = fig_image_resize(image, 320, 320);
    preprocess(image_resized, input);

    time_t start = clock();
    fig_model_forward(model);
    time_t end = clock();

    output = fig_model_output(model);
    
    for (uint32_t x = 0; x < output->height; x++) {
        for (uint32_t y = 0; y < output->width; y++) {
            float conf = exp(fig_buffer_at(output, x, y, 5)) /
                (exp(fig_buffer_at(output, x, y, 4)) + exp(fig_buffer_at(output, x, y, 5)));

            if (conf < 0.1)
                continue;

            float anchor_x = (x + 0.5f) / output->width;
            float anchor_y = (y + 0.5f) / output->height;
            float anchor_w = 0.04f;
            float anchor_h = 0.04f;

            det[i].conf = conf;
            det[i].xc = anchor_x + 0.1f * anchor_w * fig_buffer_at(output, x, y, 0);
            det[i].yc = anchor_y + 0.1f * anchor_h * fig_buffer_at(output, x, y, 1);
            det[i].width = anchor_w * exp(0.2f * fig_buffer_at(output, x, y, 2));
            det[i].height = anchor_h * exp(0.2f * fig_buffer_at(output, x, y, 3));

            printf("%f\t%f\t%f\t%f\t%e\n", det[i].xc, det[i].yc, det[i].width, det[i].height, det[i].conf);
            
            i++;

        }
    }

    qsort(det, 20 * 20, sizeof(struct Detection), compare);

// fig_buffer_print(input, 1);
// fig_buffer_print(fig_model_output(model), 1);

    printf("Number of detections: %d\n", i);

    printf("Inference time: %f\n", (end - start) / (float) CLOCKS_PER_SEC);

    fig_buffer_destroy(input);
    fig_image_destroy(image);
    fig_model_destroy(model);

    return 0;
}
