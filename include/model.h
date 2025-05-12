#ifndef _FIG_MODEL_H_
#define _FIG_MODEL_H_

#include "layer.h"
#include "list.h"

typedef struct
{
    FigList *layers;

    FigBuffer *input_buffer,
              *output_buffer;
} FigModel;

#define fig_model_output(model) \
    (model->output_buffer)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

FigModel *fig_model_new       (FigBuffer *input_buffer);
FigModel *fig_model_from_file (const char *file_path, FigBuffer *input_buffer);
void      fig_model_add_layer (FigModel *model, FigLayer *layer);
void      fig_model_forward   (FigModel *model);
void      fig_model_destroy   (FigModel *model);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _FIG_MODEL_H_ */
