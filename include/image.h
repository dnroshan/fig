#ifndef _FIG_IMAGE_H_
#define _FIG_IMAGE_H_

#include <stdint.h>

typedef struct
{
    uint32_t width,
             height;
    unsigned char *data;
} FigImage;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

FigImage *fig_image_new     (uint32_t width, uint32_t height);
void      fig_image_destroy (FigImage *image);

FigImage *fig_image_read    (const char *file_path);
FigImage *fig_image_resize  (FigImage *image, uint32_t width, uint32_t height);
void      fig_image_write   (FigImage *image, const char *file_path);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _FIG_IMAGE_H_ */
