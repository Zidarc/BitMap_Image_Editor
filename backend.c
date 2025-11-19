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


// ------------------------------------------------------
//  Load BMP image
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

    // -------------------------------
    // Correct allocation for 2D array
    // -------------------------------
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
//  Save BMP image
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
//  Apply filter using filters.c
// ------------------------------------------------------
void backend_apply_filter(int filterID)
{
    if (!pixelArray) return;

    switch (filterID)
    {
        case 0: /* Resize */ break;  // will require user input
        case 1: /* Rotate 90° */ break;
        case 2: /* Rotate 180° */ break;
        case 3: /* Flip Horizontal */ break;
        case 4: /* Flip Vertical */ break;

        case 5: /* Grayscale */
            grayscale(imgHeight, imgWidth, pixelArray);
            break;

        case 6: /* Invert */
            invert_colors(imgHeight, imgWidth, pixelArray);
            break;

        // Other filters like blur, sepia, brightness etc. are skipped until GUI can provide input
        default: break;
    }
}


// ------------------------------------------------------
//  Template placeholder
// ------------------------------------------------------
void backend_apply_template(int templateID)
{
    // Currently disabled
    (void)templateID; // suppress unused warning
}


// ------------------------------------------------------
//  Get image dimensions
// ------------------------------------------------------
int backend_get_width()  { return imgWidth; }
int backend_get_height() { return imgHeight; }
