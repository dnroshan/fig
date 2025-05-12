/*
 * File: layer.h
 * Copyright (C) Dilnavas Roshan 2024
 * Desc:
 * 
 */

#ifndef _FIG_LAYER_H_
#define _FIG_LAYER_H_

#include <stdbool.h>
#include "buffer.h"

enum FigLayerType
{
    FIG_LAYER_CONV,
    FIG_LAYER_MAXPOOL,
    FIG_LAYER_INPUT
};

enum FigActivation
{
    FIG_ACT_NOACT,
    FIG_ACT_RELU,
    FIG_ACT_SIGMOID
};

typedef struct FigLayer FigLayer;

struct FigLayer
{
    int type;

    int activation;
    bool batchnorm;

    FigBuffer *in_buffer,
              *out_buffer;

    float *gamma,
          *beta;

    float *running_mean,
          *running_var;

    void (*forward) (FigLayer *layer);

    void (*destroy) (FigLayer *layer);
};

typedef struct
{
    FigLayer base;

    uint32_t channels;

    uint32_t kernel_w,
             kernel_h;

    uint32_t stride_x,
             stride_y;

    uint32_t padding_top,
             padding_left,
             padding_bottom,
             padding_right;

    float *bias,
          *weight;
} FigConv;

typedef struct
{
    FigLayer base;

    uint32_t kernel_w,
             kernel_h;

    uint32_t stride_x,
             stride_y;

    uint32_t padding_top,
             padding_left,
             padding_bottom,
             padding_right;
} FigMaxPool;

struct ConvDesc
{
    uint32_t channels;

    uint32_t kernel_w,
             kernel_h;

    uint32_t stride_x,
             stride_y;

    uint32_t padding_top,
             padding_left,
             padding_bottom,
             padding_right;

    float *weight,
          *bias;
};

struct BatchNormDesc
{
    float *gamma,
          *beta;

    float *running_mean,
          *running_var;
};

struct MaxPoolDesc
{
    uint32_t channels;

    uint32_t kernel_w,
             kernel_h;

    uint32_t stride_x,
             stride_y;

    uint32_t padding_top,
             padding_left,
             padding_bottom,
             padding_right;
};

#define fig_layer_output(layer) (layer->out_buffer)
#define fig_layer_forward(layer) ((*layer->forward)(layer))

FigLayer *fig_layer_conv_new    (FigBuffer *in_buffer, int activation,
                                 bool batchnorm, struct ConvDesc *conv_desc,
                                 struct BatchNormDesc *batchnorm_desc);

FigLayer *fig_layer_maxpool_new (FigBuffer *in_buffer, struct MaxPoolDesc *maxpool_desc);

void     fig_layer_destroy      (FigLayer *layer);

#endif /* _FIG_LAYER_H_ */
