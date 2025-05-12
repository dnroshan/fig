#define _ISOC99_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <assert.h>
#include "misc.h"
#include "layer.h"

#define EPSILON 1e-5


/*
 * A macro to compute width 
 * and height of convolution output 
 */

#define CONV_SIZE(in_size, kern_size, pad1, pad2, stride) \
    ((in_size - kern_size + pad1 + pad2) / stride + 1)

#define OFFSET_OF(width, channels, x, y, c) \
    ((y * width + x) * channels + c)

/* Forward propogration functions */

static void conv_forward_direct(FigLayer *layer);
static void maxpool_forward(FigLayer *layer);

/* Activation functions */

static inline float relu(float x) __attribute__((always_inline));
static inline float logistic(float x) __attribute__((always_inline));

static void conv_layer_destroy(FigLayer *layer);

FigLayer *
fig_layer_conv_new(FigBuffer *in_buffer, int activation, bool batchnorm,
                   struct ConvDesc *conv_desc,
                   struct BatchNormDesc *batchnorm_desc)
{
    FigConv *layer;
    FigLayer *base;
    uint32_t buff_width, buff_height;

    layer = malloc(sizeof *layer);
    if (!layer)
        fig_panic("failed allocating memory");

    base = (FigLayer *) layer;

    base->type = FIG_LAYER_CONV;
    base->in_buffer = in_buffer;
    base->activation = activation;
    base->batchnorm = batchnorm;
    base->forward = &conv_forward_direct;
    base->destroy = &conv_layer_destroy;

    if (base->batchnorm) {
        base->gamma = batchnorm_desc->gamma;
        base->beta = batchnorm_desc->beta;
        base->running_mean = batchnorm_desc->running_mean;
        base->running_var = batchnorm_desc->running_var;
    }

    layer->channels = conv_desc->channels;
    layer->kernel_w = conv_desc->kernel_w;
    layer->kernel_h = conv_desc->kernel_h;
    layer->stride_x = conv_desc->stride_x;
    layer->stride_y = conv_desc->stride_y;
    layer->padding_top = conv_desc->padding_top;
    layer->padding_left = conv_desc->padding_left;
    layer->padding_bottom = conv_desc->padding_bottom;
    layer->padding_right = conv_desc->padding_right;
    layer->weight = conv_desc->weight;
    layer->bias = conv_desc->bias;

    buff_width = CONV_SIZE(in_buffer->width, layer->kernel_w,
                          layer->padding_left, layer->padding_right, layer->stride_x);

    buff_height = CONV_SIZE(in_buffer->height, layer->kernel_h,
                           layer->padding_top, layer->padding_bottom, layer->stride_y);

    base->out_buffer = fig_buffer_new(buff_width, buff_height, layer->channels);

    return base;
}

FigLayer *
fig_layer_maxpool_new(FigBuffer *in_buffer, struct MaxPoolDesc *maxpool_desc)
{
    FigMaxPool *layer;
    FigLayer *base;
    uint32_t buff_width, buff_height;

    layer = malloc(sizeof *layer);
    if (!layer)
        fig_panic("failed allocating memory");

    base = (FigLayer *) layer;

    base->type = FIG_LAYER_MAXPOOL;
    base->in_buffer = in_buffer;
    base->activation = FIG_ACT_NOACT;
    base->batchnorm = false;
    base->forward = &maxpool_forward;
    base->destroy = NULL;

    layer->kernel_w = maxpool_desc->kernel_w;
    layer->kernel_h = maxpool_desc->kernel_h;
    layer->stride_x = !maxpool_desc->stride_x ? maxpool_desc->kernel_w : maxpool_desc->stride_x;
    layer->stride_y = !maxpool_desc->stride_y ? maxpool_desc->kernel_h : maxpool_desc->stride_y;
    layer->padding_top = maxpool_desc->padding_top;
    layer->padding_left = maxpool_desc->padding_left;
    layer->padding_bottom = maxpool_desc->padding_bottom;
    layer->padding_right = maxpool_desc->padding_right;

    buff_width = CONV_SIZE(in_buffer->width, layer->kernel_w,
                          layer->padding_left, layer->padding_right, layer->stride_x);
    buff_height = CONV_SIZE(in_buffer->height, layer->kernel_h,
                           layer->padding_top, layer->padding_bottom, layer->stride_y);

    base->out_buffer = fig_buffer_new(buff_width, buff_height, in_buffer->channels);

    return base;
}

void
fig_layer_destroy(FigLayer *layer)
{
    if (layer->batchnorm) {
        free(layer->gamma);
        free(layer->beta);
        free(layer->running_mean);
        free(layer->running_var);
    }

    if (layer->destroy)
        (*layer->destroy)(layer);

    fig_buffer_destroy(layer->out_buffer);
    free(layer);
}

FigLayer *
fig_input_new(uint32_t width, uint32_t height)
{
    FigLayer *layer;

    layer = malloc(sizeof *layer);
    if (!layer)
        fig_panic("failed to allocate memeory");

    layer->type = FIG_LAYER_INPUT;

    layer->activation = FIG_ACT_NOACT;
    layer->batchnorm = false;

    return layer;
}

static void
conv_forward_direct(FigLayer *layer)
{
    FigConv *conv_layer = (FigConv *) layer;

    FigBuffer *in_buffer = layer->in_buffer;
    FigBuffer *out_buffer = layer->out_buffer;

    float *kernel, k, v, accum;
    uint32_t src_x, src_y;

    for (uint32_t out_c = 0; out_c < out_buffer->channels; out_c++) {
        kernel = conv_layer->weight + conv_layer->kernel_h *
            conv_layer->kernel_w * in_buffer->channels * out_c;
        for (uint32_t y = 0; y < out_buffer->height; y++) {
            for (uint32_t x = 0; x < out_buffer->width; x++) {
                accum = 0;
                for (uint32_t ky = 0; ky < conv_layer->kernel_h; ky++) {
                    for (uint32_t kx = 0; kx < conv_layer->kernel_w; kx++) {
                        src_x = x * conv_layer->stride_x + kx;
                        src_y = y * conv_layer->stride_y + ky;

                        src_x -= conv_layer->padding_left;
                        src_y -= conv_layer->padding_top;

                        if (src_x < 0 || src_y < 0 ||
                            src_x >= in_buffer->width || src_y >= in_buffer->height)
                            continue;

                        for (uint32_t in_c = 0; in_c < in_buffer->channels; in_c++) {
                            k = kernel[OFFSET_OF(conv_layer->kernel_w,
                                    in_buffer->channels, kx, ky, in_c)]; 
                            v = fig_buffer_at(in_buffer, src_x, src_y, in_c);
                            accum += v * k;
                        }

                    }
                }
                v  = accum + conv_layer->bias[out_c];
                if (layer->batchnorm) {
                    v = (v - layer->running_mean[out_c]) / 
                        sqrtf(layer->running_var[out_c] + EPSILON);
                    v = v * layer->gamma[out_c] + layer->beta[out_c];
                }

                switch (layer->activation) {
                case FIG_ACT_NOACT:
                    break;
                case FIG_ACT_RELU:
                    v = relu(v);
                    break;
                default:
                    fig_panic("unknown activation");
                    break;
                }

                fig_buffer_at(out_buffer, x, y, out_c) = v;
            }
        }
    }
}

static void
conv_forward_gemm(FigLayer *layer)
{
    FigConv *conv_layer = (FigConv *) layer;

    FigBuffer *in_buffer = layer->in_buffer;
    FigBuffer *out_buffer = layer->out_buffer;

    float *kernel, k, v, accum;
    uint32_t src_x, src_y;

    for (uint32_t out_c = 0; out_c < out_buffer->channels; out_c++) {
        kernel = conv_layer->weight + conv_layer->kernel_h *
            conv_layer->kernel_w * in_buffer->channels * out_c;
        for (uint32_t y = 0; y < out_buffer->height; y++) {
            for (uint32_t x = 0; x < out_buffer->width; x++) {
                accum = 0;
                for (uint32_t ky = 0; ky < conv_layer->kernel_h; ky++) {
                    for (uint32_t kx = 0; kx < conv_layer->kernel_w; kx++) {
                        src_x = x * conv_layer->stride_x + kx;
                        src_y = y * conv_layer->stride_y + ky;

                        src_x -= conv_layer->padding_left;
                        src_y -= conv_layer->padding_top;

                        if (src_x < 0 || src_y < 0 ||
                            src_x >= in_buffer->width || src_y >= in_buffer->height)
                            continue;

                        for (uint32_t in_c = 0; in_c < in_buffer->channels; in_c++) {
                            k = kernel[OFFSET_OF(conv_layer->kernel_w,
                                    in_buffer->channels, kx, ky, in_c)]; 
                            v = fig_buffer_at(in_buffer, src_x, src_y, in_c);
                            accum += v * k;
                        }

                    }
                }
                v  = accum + conv_layer->bias[out_c];
                if (layer->batchnorm) {
                    v = (v - layer->running_mean[out_c]) / 
                        sqrt(layer->running_var[out_c] + EPSILON);
                    v = v * layer->gamma[out_c] + layer->beta[out_c];
                }

                switch (layer->activation) {
                case FIG_ACT_NOACT:
                    break;
                case FIG_ACT_RELU:
                    v = relu(v);
                    break;
                default:
                    fig_warn("unknown activation");
                    break;
                }

                fig_buffer_at(out_buffer, x, y, out_c) = v;
            }
        }
    }
}

static void
maxpool_forward(FigLayer *layer)
{
    FigMaxPool *maxpool_layer = (FigMaxPool *) layer;

    FigBuffer *in_buffer = layer->in_buffer;
    FigBuffer *out_buffer = layer->out_buffer;

    assert(in_buffer->channels == out_buffer->channels);

    float v, max;
    uint32_t src_x, src_y;

    for (uint32_t c = 0; c < out_buffer->channels; c++) {
        for (uint32_t y = 0; y < out_buffer->height; y++) {
            for (uint32_t x = 0; x < out_buffer->width; x++) {
                max = -FLT_MAX;
                for (uint32_t ky = 0; ky < maxpool_layer->kernel_h; ky++) {
                    for (uint32_t kx = 0; kx < maxpool_layer->kernel_w; kx++) {
                        src_x = x * maxpool_layer->stride_x + kx;
                        src_y = y * maxpool_layer->stride_y + ky;

                        src_x -= maxpool_layer->padding_left;
                        src_y -= maxpool_layer->padding_top;

                        if (src_x < 0 || src_y < 0 ||
                            src_x >= in_buffer->width || src_y >= in_buffer->height)
                            continue;

                        v = fig_buffer_at(in_buffer, src_x, src_y, c);
                        if (v > max)
                            max = v;
                    }
                }
                fig_buffer_at(out_buffer, x, y, c) = max;
            }
        }
    }
}

static void
conv_layer_destroy(FigLayer *layer)
{
    FigConv *conv_layer = (FigConv *) layer;

    free(conv_layer->weight);
    free(conv_layer->bias);

}

inline static float
relu(float x)
{
    return x > 0 ? x : 0;
}

inline static float
logistic(float x)
{
    return 1.0 / (1 + exp(-x));
}
