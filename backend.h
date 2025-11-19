#ifndef BACKEND_H
#define BACKEND_H

#include "./bmp.h"   // BITMAPFILEHEADER, BITMAPINFOHEADER, RGBTRIPLE

#ifdef __cplusplus
extern "C" {
#endif

// ------------------------------------------------------
//  Load BMP image from file path
//  Returns 1 on success, 0 on failure
// ------------------------------------------------------
int backend_load_image(const char *filepath);

// ------------------------------------------------------
//  Save BMP to output file
//  Returns 1 on success, 0 on failure
// ------------------------------------------------------
int backend_save_image(const char *outfile);

// ------------------------------------------------------
//  Apply a filter by ID (as selected in bitmapgui.c)
// ------------------------------------------------------
void backend_apply_filter(int filterID);

// ------------------------------------------------------
//  Apply a template (currently disabled / stub)
// ------------------------------------------------------
void backend_apply_template(int templateID);

// ------------------------------------------------------
//  Return current image dimensions
// ------------------------------------------------------
int backend_get_width(void);
int backend_get_height(void);

// ------------------------------------------------------
//  Free pixel buffer (call on shutdown)
// ------------------------------------------------------
void backend_free(void);

#ifdef __cplusplus
}
#endif

#endif
