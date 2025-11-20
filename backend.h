#ifndef BACKEND_H
#define BACKEND_H

#include "./bmp.h"   // BITMAPFILEHEADER, BITMAPINFOHEADER, RGBTRIPLE

#ifdef __cplusplus
extern "C" {
#endif

// ======================================================
// Load / Save
// ======================================================
int backend_load_image(const char *filepath);
int backend_save_image(const char *outfile);

// ======================================================
// Apply selected filter
// ======================================================
void backend_apply_filter(int filterID);
void backend_apply_template(int templateID);

// ======================================================
// Set parameters for filters
// ======================================================
void backend_set_brightness(int value);
void backend_set_resize(int newWidth, int newHeight);

// ======================================================
// Get information
// ======================================================
int backend_get_width(void);
int backend_get_height(void);

// ======================================================
// Cleanup
// ======================================================
void backend_free(void);

#ifdef __cplusplus
}
#endif

#endif
