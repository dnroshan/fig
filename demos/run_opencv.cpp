#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cmath>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include "buffer.h"
#include "model.h"


using namespace cv;

struct Detection
{
    float xc, yc;
    float height, width;
    float conf;
};


void
preprocess(Mat *image, FigBuffer *buffer)
{
    int offset;
    uint8_t *row_ptr;

    for (int y = 0; y < image->rows; y++) {
        for (int x = 0; x < image->cols; x++) {
            offset = fig_buffer_offset_of(buffer, x, y, 0);
            row_ptr = image->ptr(y);
            buffer->data[offset + 0] = *(row_ptr + 3 * x + 0) / 255.0;
            buffer->data[offset + 1] = *(row_ptr + 3 * x + 1) / 255.0;
            buffer->data[offset + 2] = *(row_ptr + 3 * x + 2) / 255.0;
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
    Mat image, image_resized;
    FigBuffer *input, *output;
    FigModel *model;
    struct Detection det[20 * 20];
    int i = 0;

    if (argc < 2)
        exit(1);

    input = fig_buffer_new(320, 320, 3);
    model = fig_model_from_file(argv[1], input);

    image = imread(argv[2]);
    resize(image, image_resized, Size(320, 320));
    preprocess(&image_resized, input);

    time_t start = clock();
    fig_model_forward(model);
    time_t end = clock();

    output = fig_model_output(model);

    
    for (uint32_t x = 0; x < output->height; x++) {
        for (uint32_t y = 0; y < output->width; y++) {
            float conf = exp(fig_buffer_at(output, x, y, 5)) /
                (exp(fig_buffer_at(output, x, y, 4)) + exp(fig_buffer_at(output, x, y, 5)));

            if (conf < 0.3)
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
    // fig_buffer_print(fig_model_output(model), -1);

    printf("Number of detections: %d\n", i);

    printf("Inference time: %f\n", (end - start) / (float) CLOCKS_PER_SEC);

    fig_buffer_destroy(input);
    fig_model_destroy(model);

    return 0;
}
