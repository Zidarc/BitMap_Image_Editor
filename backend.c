// Replace your backend.c with this corrected version (keeps your filters mostly intact)
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "backend.h"
#include "./bmp.h"

// =====================================================
//  GLOBAL STATE
// =====================================================

static BITMAPFILEHEADER fileheader;
static BITMAPINFOHEADER infoheader;

/* single contiguous buffer: imgHeight * imgWidth RGBTRIPLEs */
static RGBTRIPLE *pixelArray = NULL;

static int imgWidth = 0;
static int imgHeight = 0;

static int brightness_value = 50;
static float contrast_factor = 1.2f;
static int pixelate_block = 10;
int resize_width = 0;
int resize_height = 0;


// =====================================================
//  BACKEND SETTERS
// =====================================================

void backend_set_brightness(int value) {
    brightness_value = value;
}

void backend_set_resize(int newWidth, int newHeight) {
    resize_width = newWidth;
    resize_height = newHeight;
}


// =====================================================
//  HELPERS
// =====================================================

static inline int clamp(int value) {
    return (value < 0) ? 0 : (value > 255) ? 255 : value;
}

/* access pixel at row r, col c */
static inline RGBTRIPLE *px(int r, int c) {
    return &pixelArray[r * imgWidth + c];
}


// =====================================================
//  LOAD BMP (now into contiguous buffer)
// =====================================================

int backend_load_image(const char *filepath) {
    FILE *readfile = fopen(filepath, "rb");
    if (!readfile) return 0;

    if (fread(&fileheader, sizeof(BITMAPFILEHEADER), 1, readfile) != 1) { fclose(readfile); return 0; }
    if (fread(&infoheader, sizeof(BITMAPINFOHEADER), 1, readfile) != 1) { fclose(readfile); return 0; }

    if (fileheader.bfType != 0x4D42 || fileheader.bfOffBits != 54 ||
        infoheader.biSize != 40 || infoheader.biBitCount != 24 ||
        infoheader.biCompression != 0) {
        fclose(readfile);
        return 0;
    }

    imgWidth = infoheader.biWidth;
    imgHeight = abs(infoheader.biHeight);

    /* free any previous buffer */
    if (pixelArray) { free(pixelArray); pixelArray = NULL; }

    pixelArray = malloc((size_t)imgWidth * imgHeight * sizeof(RGBTRIPLE));
    if (!pixelArray) { fclose(readfile); return 0; }

    int padding = (4 - ((imgWidth * 3) % 4)) % 4;

    for (int i = 0; i < imgHeight; i++) {
        /* BMP stores rows bottom-to-top when biHeight > 0 (we used abs above) */
        if (fread(&pixelArray[i * imgWidth], sizeof(RGBTRIPLE), imgWidth, readfile) != (size_t)imgWidth) {
            free(pixelArray); pixelArray = NULL; fclose(readfile); return 0;
        }
        if (padding) fseek(readfile, padding, SEEK_CUR);
    }

    fclose(readfile);
    return 1;
}


// =====================================================
//  SAVE BMP (from contiguous buffer)
// =====================================================

int backend_save_image(const char *outfile) {
    if (!pixelArray) return 0;

    FILE *writefile = fopen(outfile, "wb");
    if (!writefile) return 0;

    int padding = (4 - ((imgWidth * 3) % 4)) % 4;
    infoheader.biSizeImage = ((imgWidth * sizeof(RGBTRIPLE)) + padding) * imgHeight;
    fileheader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + infoheader.biSizeImage;

    if (fwrite(&fileheader, sizeof(BITMAPFILEHEADER), 1, writefile) != 1) { fclose(writefile); return 0; }
    if (fwrite(&infoheader, sizeof(BITMAPINFOHEADER), 1, writefile) != 1) { fclose(writefile); return 0; }

    BYTE padValue = 0x00;
    for (int i = 0; i < imgHeight; i++) {
        if (fwrite(&pixelArray[i * imgWidth], sizeof(RGBTRIPLE), imgWidth, writefile) != (size_t)imgWidth) {
            fclose(writefile); return 0;
        }
        if (padding) fwrite(&padValue, sizeof(BYTE), padding, writefile);
    }

    fclose(writefile);
    return 1;
}


// =====================================================
//  FILTERS (use VLA by casting the contiguous buffer when calling)
// =====================================================

// grayscale
void grayscale(int h, int w, RGBTRIPLE img[h][w]) {
    for (int i=0;i<h;i++)
        for (int j=0;j<w;j++) {
            int avg = (int)round((img[i][j].rgbtRed + img[i][j].rgbtGreen + img[i][j].rgbtBlue) / 3.0);
            img[i][j].rgbtRed = img[i][j].rgbtGreen = img[i][j].rgbtBlue = avg;
        }
}

// sepia
void sepia(int h, int w, RGBTRIPLE img[h][w]) {
    for (int i=0;i<h;i++)
        for (int j=0;j<w;j++) {
            int r = img[i][j].rgbtRed;
            int g = img[i][j].rgbtGreen;
            int b = img[i][j].rgbtBlue;

            img[i][j].rgbtRed   = clamp((int)round(0.393*r + 0.769*g + 0.189*b));
            img[i][j].rgbtGreen = clamp((int)round(0.349*r + 0.686*g + 0.168*b));
            img[i][j].rgbtBlue  = clamp((int)round(0.272*r + 0.534*g + 0.131*b));
        }
}

// reflect (horizontal)
void reflect(int h, int w, RGBTRIPLE img[h][w]) {
    for (int i=0;i<h;i++)
        for (int j=0;j<w/2;j++) {
            RGBTRIPLE tmp = img[i][j];
            img[i][j] = img[i][w-1-j];
            img[i][w-1-j] = tmp;
        }
}

// vertical flip
void flip_vertical(int h, int w, RGBTRIPLE img[h][w]) {
    for (int j=0;j<w;j++)
        for (int i=0;i<h/2;i++) {
            RGBTRIPLE tmp = img[i][j];
            img[i][j] = img[h-1-i][j];
            img[h-1-i][j] = tmp;
        }
}

// invert colors
void invert_colors(int h, int w, RGBTRIPLE img[h][w]) {
    for (int i=0;i<h;i++)
        for (int j=0;j<w;j++) {
            img[i][j].rgbtRed   = 255 - img[i][j].rgbtRed;
            img[i][j].rgbtGreen = 255 - img[i][j].rgbtGreen;
            img[i][j].rgbtBlue = 255 - img[i][j].rgbtBlue;
        }
}

// brightness
void adjust_brightness(int h, int w, RGBTRIPLE img[h][w], int b) {
    for (int i=0;i<h;i++)
        for (int j=0;j<w;j++) {
            img[i][j].rgbtRed   = clamp(img[i][j].rgbtRed + b);
            img[i][j].rgbtGreen = clamp(img[i][j].rgbtGreen + b);
            img[i][j].rgbtBlue  = clamp(img[i][j].rgbtBlue + b);
        }
}

// contrast
void adjust_contrast(int h, int w, RGBTRIPLE img[h][w], float f) {
    for (int i=0;i<h;i++)
        for (int j=0;j<w;j++) {
            img[i][j].rgbtRed   = clamp((int)round((img[i][j].rgbtRed   -128)*f +128));
            img[i][j].rgbtGreen = clamp((int)round((img[i][j].rgbtGreen -128)*f +128));
            img[i][j].rgbtBlue  = clamp((int)round((img[i][j].rgbtBlue  -128)*f +128));
        }
}

void backend_apply_template(int templateID)
{
    (void)templateID;
}

// pixelate
void pixelate(int h, int w, RGBTRIPLE img[h][w], int block) {
    for (int i=0;i<h;i+=block)
        for (int j=0;j<w;j+=block) {
            int r=0,g=0,b=0,count=0;

            for (int bi=i; bi < (i+block && i+block<h ? i+block : h); bi++)
                for (int bj=j; bj < (j+block && j+block<w ? j+block : w); bj++) {
                    r += img[bi][bj].rgbtRed;
                    g += img[bi][bj].rgbtGreen;
                    b += img[bi][bj].rgbtBlue;
                    count++;
                }

            if (count==0) continue;
            int ar = r/count, ag = g/count, ab = b/count;

            for (int bi=i; bi < (i+block && i+block<h ? i+block : h); bi++)
                for (int bj=j; bj < (j+block && j+block<w ? j+block : w); bj++) {
                    img[bi][bj].rgbtRed = ar;
                    img[bi][bj].rgbtGreen = ag;
                    img[bi][bj].rgbtBlue = ab;
                }
        }
}

// blur
void blur(int h, int w, RGBTRIPLE img[h][w]) {
    RGBTRIPLE *tmp = malloc((size_t)h * w * sizeof(RGBTRIPLE));
    if (!tmp) return;

    for (int i=0;i<h;i++)
        for (int j=0;j<w;j++) {
            int r=0,g=0,b=0,c=0;
            for (int di=-1; di<=1; di++)
                for (int dj=-1; dj<=1; dj++) {
                    int ni=i+di, nj=j+dj;
                    if (ni>=0 && nj>=0 && ni<h && nj<w) {
                        RGBTRIPLE *p = &img[ni][nj];
                        r += p->rgbtRed;
                        g += p->rgbtGreen;
                        b += p->rgbtBlue;
                        c++;
                    }
                }
            tmp[i*w + j].rgbtRed = c? r/c:0;
            tmp[i*w + j].rgbtGreen = c? g/c:0;
            tmp[i*w + j].rgbtBlue = c? b/c:0;
        }

    memcpy(img, tmp, (size_t)h*w*sizeof(RGBTRIPLE));
    free(tmp);
}





// =====================================================
//  ROTATE / RESIZE (operate on contiguous buffer and update it)
// =====================================================

/* rotate 90 degrees clockwise: allocate new buffer, fill, free old, update globals */
void rotate_90_contiguous(int *h, int *w, RGBTRIPLE **buf_ptr) {
    int oldH = *h, oldW = *w;
    int newH = oldW, newW = oldH;

    RGBTRIPLE *newbuf = malloc((size_t)newH * newW * sizeof(RGBTRIPLE));
    if (!newbuf) return;

    for (int i=0;i<oldH;i++)
        for (int j=0;j<oldW;j++) {
            /* pixel at (i,j) goes to (j, newW-1-i) */
            newbuf[j * newW + (newW - 1 - i)] = (*buf_ptr)[i * oldW + j];
        }

    free(*buf_ptr);
    *buf_ptr = newbuf;
    *h = newH; *w = newW;
}

/* rotate 180 in-place on contiguous buffer */
void rotate_180_contiguous(int h, int w, RGBTRIPLE *buf) {
    for (int i=0;i<h/2;i++)
        for (int j=0;j<w;j++) {
            RGBTRIPLE tmp = buf[i*w + j];
            buf[i*w + j] = buf[(h-1-i)*w + (w-1-j)];
            buf[(h-1-i)*w + (w-1-j)] = tmp;
        }
}

/* resize using nearest-neighbour, returns new buffer and updates headers */
int resize_contiguous(int *h, int *w, RGBTRIPLE **buf_ptr, int newW, int newH) {
    if (newW <= 0 || newH <= 0) return 0;

    RGBTRIPLE *newbuf = malloc((size_t)newW * newH * sizeof(RGBTRIPLE));
    if (!newbuf) return 0;

    for (int i=0;i<newH;i++)
        for (int j=0;j<newW;j++) {
            int oi = i * (*h) / newH;
            int oj = j * (*w) / newW;
            newbuf[i*newW + j] = (*buf_ptr)[oi * (*w) + oj];
        }

    free(*buf_ptr);
    *buf_ptr = newbuf;
    *h = newH; *w = newW;

    /* update headers (fileheader/infoheader are globals) */
    int padding = (4 - ((newW * sizeof(RGBTRIPLE)) % 4)) % 4;
    infoheader.biWidth = newW;
    infoheader.biHeight = newH;
    infoheader.biSizeImage = (sizeof(RGBTRIPLE) * newW + padding) * newH;
    fileheader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + infoheader.biSizeImage;
    backend_save_image("resized.bmp");
    return 1;
}


// =====================================================
//  APPLY FILTER
// =====================================================

void backend_apply_filter(int filterID) {
    if (!pixelArray) return;

    switch(filterID) {
        case 13: // Resize (we use resize_width/height)
            if (resize_width > 0 && resize_height > 0) {
                resize_contiguous(&imgHeight, &imgWidth, &pixelArray, resize_width, resize_height);
            }
            break;

        case 0:
            rotate_90_contiguous(&imgHeight, &imgWidth, &pixelArray);
            break;

        case 1:
            rotate_180_contiguous(imgHeight, imgWidth, pixelArray);
            break;

        case 2:
            reflect(imgHeight, imgWidth, (RGBTRIPLE(*)[imgWidth])pixelArray);
            break;
        case 3:
            flip_vertical(imgHeight, imgWidth, (RGBTRIPLE(*)[imgWidth])pixelArray);
            break;
        case 4:
            grayscale(imgHeight, imgWidth, (RGBTRIPLE(*)[imgWidth])pixelArray);
            break;
        case 5:
            invert_colors(imgHeight, imgWidth, (RGBTRIPLE(*)[imgWidth])pixelArray);
            break;
        case 6:
            sepia(imgHeight, imgWidth, (RGBTRIPLE(*)[imgWidth])pixelArray);
            break;
        case 7:
            blur(imgHeight, imgWidth, (RGBTRIPLE(*)[imgWidth])pixelArray);
            break;

        case 11:
            adjust_brightness(imgHeight, imgWidth, (RGBTRIPLE(*)[imgWidth])pixelArray, brightness_value);
            break;
        case 8:
            adjust_contrast(imgHeight, imgWidth, (RGBTRIPLE(*)[imgWidth])pixelArray, contrast_factor);
            break;
        case 9:
            pixelate(imgHeight, imgWidth, (RGBTRIPLE(*)[imgWidth])pixelArray, pixelate_block);
            break;

        default:
            break;
    }
}


// =====================================================
//  GETTERS
// =====================================================

int backend_get_width()  { return imgWidth; }
int backend_get_height() { return imgHeight; }


// =====================================================
//  CLEANUP
// =====================================================

void backend_free(void) {
    if (!pixelArray) return;
    free(pixelArray);
    pixelArray = NULL;
    imgWidth = imgHeight = 0;
}
