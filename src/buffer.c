#include <stdlib.h>
#include "misc.h"
#include "buffer.h"

FigBuffer *
fig_buffer_new(uint32_t width, uint32_t height, uint32_t channels)
{
    FigBuffer *buffer;

    buffer = malloc(sizeof *buffer);
    if (!buffer)
        fig_panic("Failed allocating memory");

    buffer->width = width;
    buffer->height = height;
    buffer->channels = channels;
    buffer->data = malloc(width * height * channels * sizeof(float));

    return buffer;
}

void
fig_buffer_destroy(FigBuffer *buffer)
{
    free(buffer->data);
    free(buffer);
    buffer = NULL;
}

#ifndef NDEBUG

void
fig_buffer_print(FigBuffer *buffer, int rows)
{
    uint32_t max_rows = rows < 0 ? buffer->height : (uint32_t) rows;

    for (uint32_t y = 0; y < max_rows; y++) {
        for (uint32_t x = 0; x < buffer->width; x++) {
            for (uint32_t c = 0; c < buffer->channels; c++) {
                if (c) printf(" ");
                printf("%.3f", fig_buffer_at(buffer, x, y, c));
            }
            printf("\n");
        }
        printf("\n\n");
    }
    printf("width: %d height: %d channels: %d\n",
            buffer->width, buffer->height, buffer->channels);
}

#endif /* NDEBUG */
