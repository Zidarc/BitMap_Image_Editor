#include "backend.h"
#include "filters.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./bmp.h"

static BITMAPFILEHEADER fileheader;
static BITMAPINFOHEADER infoheader;

// Corrected type: pointer-to-pointer 2D dynamic array
static RGBTRIPLE **pixelArray = NULL;

static int imgWidth = 0;
static int imgHeight = 0;

// Adjustable parameters (can be set via GUI later)
static int brightness_value = 50;
static float contrast_factor = 1.2f;
static int pixelate_block = 10;
static int resize_width = 0;   // set >0 to enable resize
static int resize_height = 0;

// Add near the top with the static variables
void backend_set_brightness(int value) {
    brightness_value = value;
}

void backend_set_resize(int newWidth, int newHeight) {
    resize_width = newWidth;
    resize_height = newHeight;
}


// ------------------------------------------------------
// Load BMP image
// ------------------------------------------------------
int backend_load_image(const char *filepath)
{
    FILE *readfile = fopen(filepath, "rb");
    if (!readfile) return 0;

    fread(&fileheader, sizeof(BITMAPFILEHEADER), 1, readfile);
    fread(&infoheader, sizeof(BITMAPINFOHEADER), 1, readfile);

    if (fileheader.bfType != 0x4D42 || fileheader.bfOffBits != 54 ||
        infoheader.biSize != 40 || infoheader.biBitCount != 24 || infoheader.biCompression != 0)
    {
        fclose(readfile);
        return 0;
    }

    imgWidth = infoheader.biWidth;
    imgHeight = abs(infoheader.biHeight);

    // Allocate 2D pixel array
    pixelArray = malloc(imgHeight * sizeof(RGBTRIPLE *));
    if (!pixelArray) { fclose(readfile); return 0; }

    for (int i = 0; i < imgHeight; i++)
    {
        pixelArray[i] = malloc(imgWidth * sizeof(RGBTRIPLE));
        if (!pixelArray[i]) return 0;
    }

    int padding = (4 - ((imgWidth * 3) % 4)) % 4;

    for (int i = 0; i < imgHeight; i++)
    {
        fread(pixelArray[i], sizeof(RGBTRIPLE), imgWidth, readfile);
        fseek(readfile, padding, SEEK_CUR);
    }

    fclose(readfile);
    return 1;
}

// ------------------------------------------------------
// Save BMP image
// ------------------------------------------------------
int backend_save_image(const char *outfile)
{
    if (!pixelArray) return 0;

    FILE *writefile = fopen(outfile, "wb");
    if (!writefile) return 0;

    int padding = (4 - ((imgWidth * 3) % 4)) % 4;
    infoheader.biSizeImage = ((imgWidth * sizeof(RGBTRIPLE)) + padding) * imgHeight;
    fileheader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + infoheader.biSizeImage;

    fwrite(&fileheader, sizeof(BITMAPFILEHEADER), 1, writefile);
    fwrite(&infoheader, sizeof(BITMAPINFOHEADER), 1, writefile);

    BYTE padValue = 0x00;
    for (int i = 0; i < imgHeight; i++)
    {
        fwrite(pixelArray[i], sizeof(RGBTRIPLE), imgWidth, writefile);
        fwrite(&padValue, sizeof(BYTE), padding, writefile);
    }

    fclose(writefile);
    return 1;
}

// ------------------------------------------------------
// Apply filter using filters.c
// ------------------------------------------------------
void backend_apply_filter(int filterID)
{
    if (!pixelArray) return;

    int padding = (4 - (imgWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    switch (filterID)
    {
        case 0: /* Resize */
            if (resize_width > 0 && resize_height > 0)
                resize(&imgHeight, &imgWidth, &padding, (RGBTRIPLE (**)[imgWidth])pixelArray,
                       resize_width, resize_height, &fileheader, &infoheader);
            break;

        case 1: /* Rotate 90° */
            rotate_90(&imgHeight, &imgWidth, &padding, (RGBTRIPLE (**)[imgWidth])pixelArray);
            break;

        case 2: /* Rotate 180° */
            rotate_180(imgHeight, imgWidth, pixelArray);
            break;

        case 3: /* Flip Horizontal */
            reflect(imgHeight, imgWidth, pixelArray);
            break;

        case 4: /* Flip Vertical */
            flip_vertical(imgHeight, imgWidth, pixelArray); // implement in filters.c
            break;

        case 5: /* Grayscale */
            grayscale(imgHeight, imgWidth, pixelArray);
            break;

        case 6: /* Invert */
            invert_colors(imgHeight, imgWidth, pixelArray);
            break;

        case 7: /* Sepia */
            sepia(imgHeight, imgWidth, pixelArray);
            break;

        case 8: /* Blur */
            blur(imgHeight, imgWidth, pixelArray);
            break;

        case 9: /* Edges */
            edges(imgHeight, imgWidth, pixelArray); // optional
            break;

        case 10: /* Brightness */
            adjust_brightness(imgHeight, imgWidth, pixelArray, brightness_value);
            break;

        case 11: /* Contrast */
            adjust_contrast(imgHeight, imgWidth, pixelArray, contrast_factor);
            break;

        case 12: /* Pixelate */
            pixelate(imgHeight, imgWidth, pixelArray, pixelate_block);
            break;

        default:
            break;
    }
}

// ------------------------------------------------------
// Apply template (stub)
// ------------------------------------------------------
void backend_apply_template(int templateID)
{
    (void)templateID; // suppress unused warning
}

// ------------------------------------------------------
// Get image dimensions
// ------------------------------------------------------
int backend_get_width()  { return imgWidth; }
int backend_get_height() { return imgHeight; }

// ------------------------------------------------------
// Free pixel buffer (call on shutdown)
// ------------------------------------------------------
void backend_free(void) {
    if (!pixelArray) return;
    for (int i = 0; i < imgHeight; i++) free(pixelArray[i]);
    free(pixelArray);
    pixelArray = NULL;
    imgWidth = imgHeight = 0;
}

