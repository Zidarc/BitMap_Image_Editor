#ifndef BACKEND_H
#define BACKEND_H

#include "./bmp.h"   // BITMAPFILEHEADER, BITMAPINFOHEADER, RGBTRIPLE

#ifdef __cplusplus
extern "C" {
#endif

// ------------------------------------------------------
// Load BMP image from file path
// Returns 1 on success, 0 on failure
// ------------------------------------------------------
int backend_load_image(const char *filepath);

// ------------------------------------------------------
// Save BMP to output file
// Returns 1 on success, 0 on failure
// ------------------------------------------------------
int backend_save_image(const char *outfile);

// ------------------------------------------------------
// Apply a filter by ID (as selected in bitmapgui.c)
// Filters: 0=Resize,1=Rotate90,2=Rotate180,3=FlipH,4=FlipV,
// 5=Grayscale,6=Invert,7=Sepia,8=Blur,9=Edges,
// 10=Brightness,11=Contrast,12=Pixelate
// ------------------------------------------------------
void backend_apply_filter(int filterID);
    
// ------------------------------------------------------
// Apply a template (currently disabled / stub)
// ------------------------------------------------------
void backend_apply_template(int templateID);

// ------------------------------------------------------
// Return current image dimensions
// ------------------------------------------------------
int backend_get_width(void);
int backend_get_height(void);

// ------------------------------------------------------
// Free pixel buffer (call on shutdown)
// ------------------------------------------------------
void backend_free(void);

// ------------------------------------------------------
// Apply a filter by ID (as selected in bitmapgui.c)    
void backend_set_brightness(int value);
void backend_set_resize(int newWidth, int newHeight);
void adjust_brightness(int height, int width, RGBTRIPLE **image, int brightness);
void resize(int *height, int *width, int *padding, RGBTRIPLE (**image)[*width], int newW, int newH, BITMAPFILEHEADER *bf, BITMAPINFOHEADER *bi);

// ------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
