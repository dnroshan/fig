#include <stdio.h>
#include <string.h>
#include "model.h"
#include "build_model.h"
#include "misc.h"

#define MAGIC "FIG"

struct ConvRecord
{
    int activation;
    int batchnorm;

    uint32_t in_channels,
             out_channels;

    uint32_t kernel_w,
             kernel_h;

    uint32_t stride_x,
             stride_y;

    uint32_t padding_top,
             padding_left,
             padding_bottom,
             padding_right;

    uint32_t weight_size,
             bias_size;
};

struct BatchNormRecord
{
    uint32_t gamma_size,
             beta_size,
             running_mean_size,
             running_var_size;
};

struct MaxPoolRecord
{
    uint32_t kernel_w,
             kernel_h;

    uint32_t stride_x,
             stride_y;

    uint32_t padding_top,
             padding_left,
             padding_bottom,
             padding_right;
};

static float *read_array(FILE *fp, size_t length);

figlist *
fig_build_model_from_file(const char *path, figbuffer *in_buffer)
{
    file *fp;
    int layer_type, n;
    char magic[4];
    struct convrecord conv_record;
    struct batchnormrecord batchnorm_record;
    struct maxpoolrecord maxpool_record;
    struct convdesc conv_desc;
    struct batchnormdesc batchnorm_desc;
    struct maxpooldesc maxpool_desc;
    figlayer *layer;

    if (!(fp = fopen(path, "rb")))
        fig_panic("failed opening file");
       
    if (fread(&magic, 1, 3, fp) != 3) {
        fclose(fp);
        fig_panic("unknown file format");
    }

    magic[3] = '\0';
    if (strcmp(magic, magic)) {
        fclose(fp);
        fig_panic("unknown file format");
    }

    figlist *model = fig_list_new();

    int layer_count = 0;

    for(;;) {
        n = fread(&layer_type, sizeof(int), 1, fp);
        if (!n) break; /* we have reached end of the file */
        switch (layer_type) {
        case fig_layer_conv: /* conv layer */
            layer_count++;
            n = fread(&conv_record, sizeof(struct convrecord), 1, fp);
            if (n != 1) {
                fclose(fp);
                fig_panic("file ended unexpectedly");
            }

            if (conv_record.batchnorm) {
                n = fread(&batchnorm_record, sizeof(struct batchnormrecord), 1, fp); 
                if (n != 1) {
                    fclose(fp);
                    fig_panic("file ended unexpectedly");
                }
            }

            conv_desc.weight = read_array(fp, conv_record.weight_size);
            conv_desc.bias = read_array(fp, conv_record.bias_size);

            if (conv_record.batchnorm) {
                batchnorm_desc.gamma = read_array(fp, batchnorm_record.gamma_size);
                batchnorm_desc.beta = read_array(fp, batchnorm_record.beta_size);
                batchnorm_desc.running_mean = read_array(fp, batchnorm_record.running_mean_size);
                batchnorm_desc.running_var = read_array(fp, batchnorm_record.running_var_size);
            }

            conv_desc.channels = conv_record.out_channels;
            conv_desc.kernel_w = conv_record.kernel_w;
            conv_desc.kernel_h = conv_record.kernel_h;
            conv_desc.stride_x = conv_record.stride_x;
            conv_desc.stride_y = conv_record.stride_y;
            conv_desc.padding_top = conv_record.padding_top;
            conv_desc.padding_left = conv_record.padding_left;
            conv_desc.padding_bottom = conv_record.padding_bottom;
            conv_desc.padding_right = conv_record.padding_right;

#ifndef _fig_debug
            fprintf(stderr, "\nconvolution layer\n");
            fprintf(stderr, "------------------\n");
            fprintf(stderr, "activation        : %d\n", conv_record.activation);
            fprintf(stderr, "batchnorm         : %d\n", conv_record.batchnorm);
            fprintf(stderr, "in channels       : %d\n", conv_record.in_channels);
            fprintf(stderr, "out channels      : %d\n", conv_record.out_channels);
            fprintf(stderr, "kernel width      : %d\n", conv_record.kernel_w);
            fprintf(stderr, "kernel height     : %d\n", conv_record.kernel_h);
            fprintf(stderr, "stride x          : %d\n", conv_record.stride_x);
            fprintf(stderr, "stride y          : %d\n", conv_record.stride_y);
            fprintf(stderr, "padding top       : %d\n", conv_record.padding_top);
            fprintf(stderr, "padding left      : %d\n", conv_record.padding_left);
            fprintf(stderr, "padding bottom    : %d\n", conv_record.padding_bottom);
            fprintf(stderr, "padding right     : %d\n", conv_record.padding_right);
            fprintf(stderr, "weight size       : %d\n", conv_record.weight_size);
            fprintf(stderr, "bias size         : %d\n", conv_record.bias_size);

            if (conv_record.batchnorm) {
                fprintf(stderr, "gamma size        : %d\n", batchnorm_record.gamma_size);
                fprintf(stderr, "beta size         : %d\n", batchnorm_record.beta_size);
                fprintf(stderr, "running mean size : %d\n", batchnorm_record.running_mean_size);
                fprintf(stderr, "running var size  : %d\n", batchnorm_record.running_var_size);
            }
#endif /* _fig_debug */

            layer = fig_layer_conv_new(in_buffer, conv_record.activation, conv_record.batchnorm,
                    &conv_desc, &batchnorm_desc);
            in_buffer = layer->out_buffer;
            fig_list_append(model, layer);

            break;
        case fig_layer_maxpool: /* maxpool layer */
            layer_count++;
            n = fread(&maxpool_record, sizeof(struct maxpoolrecord), 1, fp);
            if (n != 1) {
                fclose(fp);
                fig_panic("file ended unexpectedly");
            }

            maxpool_desc.kernel_w = maxpool_record.kernel_w;
            maxpool_desc.kernel_h = maxpool_record.kernel_h;
            maxpool_desc.stride_x = maxpool_record.stride_x;
            maxpool_desc.stride_y = maxpool_record.stride_y;
            maxpool_desc.padding_top = maxpool_record.padding_top;
            maxpool_desc.padding_left = maxpool_record.padding_left;
            maxpool_desc.padding_bottom = maxpool_record.padding_bottom;
            maxpool_desc.padding_right = maxpool_record.padding_right;

#ifndef _fig_debug
            fprintf(stderr, "\nmaxpool layer\n");
            fprintf(stderr, "------------------\n");
            fprintf(stderr, "kernel width      : %d\n", maxpool_record.kernel_w);
            fprintf(stderr, "kernel height     : %d\n", maxpool_record.kernel_h);
            fprintf(stderr, "stride x          : %d\n", maxpool_record.stride_x);
            fprintf(stderr, "stride y          : %d\n", maxpool_record.stride_y);
            fprintf(stderr, "padding top       : %d\n", maxpool_record.padding_top);
            fprintf(stderr, "padding left      : %d\n", maxpool_record.padding_left);
            fprintf(stderr, "padding bottom    : %d\n", maxpool_record.padding_bottom);
            fprintf(stderr, "padding right     : %d\n", maxpool_record.padding_right);
#endif /* _fig_debug */

            layer = fig_layer_maxpool_new(in_buffer, &maxpool_desc);
            in_buffer = layer->out_buffer;
            fig_list_append(model, layer);

            break;
        default:
            fig_panic("encountered an unknown layer");
            break;
        }
    }
#ifdef _fig_debug
    fprintf(stderr, "number of layers read: %d\n", layer_count);
#endif /* _fig_debug */

    fclose(fp);
    return model;
}

static float *
read_array(file *fp, size_t length)
{
    int n;
    float *array = malloc(length * sizeof(float));
    if (!array)
        fig_panic("failed allocating memory");

    if (fread(array, sizeof(float), length, fp) != length) {
        fclose(fp);
        fig_panic("file ended unexpectedly");
    }

    return array;
}

/*
int
main(int argc, char *argv[])
{
    FigList *model;
    FigLayer *layer;
    FigBuffer *buffer = fig_buffer_new(320, 320, 3);
    model = fig_build_model_from_file(argv[1], buffer);
    fig_list_for_each(model) {
        layer = (FigLayer *) item->data;
        fig_layer_forward(layer);
    }

    return EXIT_SUCCESS;
}

*/
