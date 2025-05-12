#ifndef _FIG_BUFFER_H_
#define _FIG_BUFFER_H_

#include <stdint.h>

typedef struct
{
    float *data;
    uint32_t height,
             width;
    uint32_t channels;
} FigBuffer;

#define fig_buffer_len(buffer) \
    (buffer->width * buffer-> height * buffer->channels)

#define fig_buffer_offset_of(buffer, x, y, c) \
    (buffer->channels * (y * buffer->width + x) + c)  

#define fig_buffer_at(buffer, x, y, c) \
    buffer->data[fig_buffer_offset_of(buffer, x, y, c)]

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

FigBuffer *fig_buffer_new     (uint32_t width, uint32_t height,
                               uint32_t channels);
void       fig_buffer_destroy (FigBuffer *buffer);

#ifndef NDEBUG

void       fig_buffer_print   (FigBuffer *buffer, int rows);

#endif /* NDEBUG */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _FIG_BUFFER_H_ */
