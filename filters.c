#include "./filters.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// -------------------- Helper functions --------------------

// Clamp value to [0, 255]
static inline int clamp(int value) {
    return fmax(fmin(value, 255), 0);
}

// -------------------- Option 1: Standard filters --------------------

void grayscale(int height, int width, RGBTRIPLE **image) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int avg = round((image[i][j].rgbtRed + image[i][j].rgbtGreen + image[i][j].rgbtBlue) / 3.0);
            image[i][j].rgbtRed = image[i][j].rgbtGreen = image[i][j].rgbtBlue = avg;
        }
    }
}

void sepia(int height, int width, RGBTRIPLE **image) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int red = image[i][j].rgbtRed;
            int green = image[i][j].rgbtGreen;
            int blue = image[i][j].rgbtBlue;

            int sepiaRed = clamp(round(0.393*red + 0.769*green + 0.189*blue));
            int sepiaGreen = clamp(round(0.349*red + 0.686*green + 0.168*blue));
            int sepiaBlue = clamp(round(0.272*red + 0.534*green + 0.131*blue));

            image[i][j].rgbtRed = sepiaRed;
            image[i][j].rgbtGreen = sepiaGreen;
            image[i][j].rgbtBlue = sepiaBlue;
        }
    }
}

void reflect(int height, int width, RGBTRIPLE **image) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width / 2; j++) {
            RGBTRIPLE temp = image[i][j];
            image[i][j] = image[i][width - 1 - j];
            image[i][width - 1 - j] = temp;
        }
    }
}

void blur(int height, int width, RGBTRIPLE **image) {
    RGBTRIPLE **temp = malloc(height * sizeof(RGBTRIPLE *));
    if (!temp) return;
    for (int i = 0; i < height; i++)
        temp[i] = malloc(width * sizeof(RGBTRIPLE));

    int dx[] = {-1, 0, 1};
    int dy[] = {-1, 0, 1};

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int r = 0, g = 0, b = 0, count = 0;
            for (int di = 0; di < 3; di++) {
                for (int dj = 0; dj < 3; dj++) {
                    int ni = i + dx[di], nj = j + dy[dj];
                    if (ni >= 0 && ni < height && nj >= 0 && nj < width) {
                        r += image[ni][nj].rgbtRed;
                        g += image[ni][nj].rgbtGreen;
                        b += image[ni][nj].rgbtBlue;
                        count++;
                    }
                }
            }
            temp[i][j].rgbtRed = round(r / (double)count);
            temp[i][j].rgbtGreen = round(g / (double)count);
            temp[i][j].rgbtBlue = round(b / (double)count);
        }
    }

    for (int i = 0; i < height; i++)
        memcpy(image[i], temp[i], width * sizeof(RGBTRIPLE));

    for (int i = 0; i < height; i++) free(temp[i]);
    free(temp);
}

void invert_colors(int height, int width, RGBTRIPLE **image) {
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++) {
            image[i][j].rgbtRed = 255 - image[i][j].rgbtRed;
            image[i][j].rgbtGreen = 255 - image[i][j].rgbtGreen;
            image[i][j].rgbtBlue = 255 - image[i][j].rgbtBlue;
        }
}

void adjust_brightness(int height, int width, RGBTRIPLE **image, int brightness) {
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++) {
            image[i][j].rgbtRed = clamp(image[i][j].rgbtRed + brightness);
            image[i][j].rgbtGreen = clamp(image[i][j].rgbtGreen + brightness);
            image[i][j].rgbtBlue = clamp(image[i][j].rgbtBlue + brightness);
        }
}

void adjust_contrast(int height, int width, RGBTRIPLE **image, float factor) {
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++) {
            image[i][j].rgbtRed = clamp(round((image[i][j].rgbtRed - 128) * factor + 128));
            image[i][j].rgbtGreen = clamp(round((image[i][j].rgbtGreen - 128) * factor + 128));
            image[i][j].rgbtBlue = clamp(round((image[i][j].rgbtBlue - 128) * factor + 128));
        }
}

void pixelate(int height, int width, RGBTRIPLE **image, int block_size) {
    for (int i = 0; i < height; i += block_size) {
        for (int j = 0; j < width; j += block_size) {
            int r = 0, g = 0, b = 0, count = 0;
            for (int bi = i; bi < fmin(i + block_size, height); bi++)
                for (int bj = j; bj < fmin(j + block_size, width); bj++) {
                    r += image[bi][bj].rgbtRed;
                    g += image[bi][bj].rgbtGreen;
                    b += image[bi][bj].rgbtBlue;
                    count++;
                }
            int avgR = round(r / (double)count);
            int avgG = round(g / (double)count);
            int avgB = round(b / (double)count);
            for (int bi = i; bi < fmin(i + block_size, height); bi++)
                for (int bj = j; bj < fmin(j + block_size, width); bj++) {
                    image[bi][bj].rgbtRed = avgR;
                    image[bi][bj].rgbtGreen = avgG;
                    image[bi][bj].rgbtBlue = avgB;
                }
        }
    }
}

void vignette(int height, int width, RGBTRIPLE **image) {
    int centerX = width / 2;
    int centerY = height / 2;
    double max_dist = sqrt(centerX*centerX + centerY*centerY);

    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++) {
            double dist = sqrt(pow(j - centerX, 2) + pow(i - centerY, 2));
            double factor = 1.0 - dist / max_dist;
            image[i][j].rgbtRed = clamp(round(image[i][j].rgbtRed * factor));
            image[i][j].rgbtGreen = clamp(round(image[i][j].rgbtGreen * factor));
            image[i][j].rgbtBlue = clamp(round(image[i][j].rgbtBlue * factor));
        }
}

void sharpen(int height, int width, RGBTRIPLE **image) {
    int kernel[3][3] = {{0,-1,0},{-1,5,-1},{0,-1,0}};
    RGBTRIPLE **temp = malloc(height * sizeof(RGBTRIPLE *));
    for (int i = 0; i < height; i++)
        temp[i] = malloc(width * sizeof(RGBTRIPLE));

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int r = 0, g = 0, b = 0;
            for (int ki = -1; ki <= 1; ki++)
                for (int kj = -1; kj <= 1; kj++) {
                    int ni = i + ki, nj = j + kj;
                    if (ni >= 0 && ni < height && nj >= 0 && nj < width) {
                        r += kernel[ki+1][kj+1] * image[ni][nj].rgbtRed;
                        g += kernel[ki+1][kj+1] * image[ni][nj].rgbtGreen;
                        b += kernel[ki+1][kj+1] * image[ni][nj].rgbtBlue;
                    }
                }
            temp[i][j].rgbtRed = clamp(r);
            temp[i][j].rgbtGreen = clamp(g);
            temp[i][j].rgbtBlue = clamp(b);
        }
    }

    for (int i = 0; i < height; i++)
        memcpy(image[i], temp[i], width * sizeof(RGBTRIPLE));

    for (int i = 0; i < height; i++) free(temp[i]);
    free(temp);
}

void gaussian_blur(int height, int width, RGBTRIPLE **image) {
    float kernel[5][5] = {
        {1,4,6,4,1},{4,16,24,16,4},{6,24,36,24,6},{4,16,24,16,4},{1,4,6,4,1}
    };
    for (int i=0;i<5;i++) for (int j=0;j<5;j++) kernel[i][j]/=256.0;

    RGBTRIPLE **temp = malloc(height * sizeof(RGBTRIPLE *));
    for (int i = 0; i < height; i++)
        temp[i] = malloc(width * sizeof(RGBTRIPLE));

    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++) {
            double r=0,g=0,b=0;
            for (int ki=-2;ki<=2;ki++)
                for (int kj=-2;kj<=2;kj++) {
                    int ni = i+ki, nj = j+kj;
                    if (ni>=0 && ni<height && nj>=0 && nj<width){
                        r += kernel[ki+2][kj+2]*image[ni][nj].rgbtRed;
                        g += kernel[ki+2][kj+2]*image[ni][nj].rgbtGreen;
                        b += kernel[ki+2][kj+2]*image[ni][nj].rgbtBlue;
                    }
                }
            temp[i][j].rgbtRed = clamp(round(r));
            temp[i][j].rgbtGreen = clamp(round(g));
            temp[i][j].rgbtBlue = clamp(round(b));
        }

    for (int i=0;i<height;i++) memcpy(image[i], temp[i], width*sizeof(RGBTRIPLE));
    for (int i=0;i<height;i++) free(temp[i]);
    free(temp);
}

void emboss(int height, int width, RGBTRIPLE **image) {
    int kernel[3][3] = {{-2,-1,0},{-1,1,1},{0,1,2}};
    RGBTRIPLE **temp = malloc(height * sizeof(RGBTRIPLE *));
    for (int i=0;i<height;i++) temp[i]=malloc(width*sizeof(RGBTRIPLE));

    for (int i=0;i<height;i++)
        for (int j=0;j<width;j++) {
            int r=0,g=0,b=0;
            for (int ki=-1;ki<=1;ki++)
                for (int kj=-1;kj<=1;kj++){
                    int ni=i+ki,nj=j+kj;
                    if(ni>=0 && ni<height && nj>=0 && nj<width){
                        r+=kernel[ki+1][kj+1]*image[ni][nj].rgbtRed;
                        g+=kernel[ki+1][kj+1]*image[ni][nj].rgbtGreen;
                        b+=kernel[ki+1][kj+1]*image[ni][nj].rgbtBlue;
                    }
                }
            temp[i][j].rgbtRed = clamp(round(r));
            temp[i][j].rgbtGreen = clamp(round(g));
            temp[i][j].rgbtBlue = clamp(round(b));
        }
    for(int i=0;i<height;i++) memcpy(image[i], temp[i], width*sizeof(RGBTRIPLE));
    for(int i=0;i<height;i++) free(temp[i]);
    free(temp);
}

void add_border(int height, int width, RGBTRIPLE **image, int border_width, RGBTRIPLE border_color) {
    for (int i=0;i<height;i++)
        for (int j=0;j<width;j++)
            if(i<border_width || i>=height-border_width || j<border_width || j>=width-border_width)
                image[i][j]=border_color;
}

// -------------------- Option 3: Resize / Rotation --------------------

void rotate_90(int *height, int *width, int *padding, RGBTRIPLE (**image)[*width]) {
    int newH = *width, newW = *height;
    int newP = (4 - (newW*sizeof(RGBTRIPLE))%4)%4;
    RGBTRIPLE (*rot)[newW] = malloc(newH * sizeof(RGBTRIPLE[newW]));
    for(int i=0;i<*height;i++)
        for(int j=0;j<*width;j++)
            rot[j][newW-1-i]=(*image)[i][j];
    free(*image);
    *image = (RGBTRIPLE (*)[newW])rot;
    *height=newH; *width=newW; *padding=newP;
}

void rotate_180(int height, int width, RGBTRIPLE **image) {
    for(int i=0;i<height/2;i++)
        for(int j=0;j<width;j++){
            RGBTRIPLE temp=image[i][j];
            image[i][j]=image[height-1-i][width-1-j];
            image[height-1-i][width-1-j]=temp;
        }
    if(height%2!=0){
        int row=height/2;
        for(int j=0;j<width/2;j++){
            RGBTRIPLE temp=image[row][j];
            image[row][j]=image[row][width-1-j];
            image[row][width-1-j]=temp;
        }
    }
}

void rotate_270(int *height, int *width, int *padding, RGBTRIPLE (**image)[*width]) {
    int newH=*width, newW=*height;
    int newP=(4-(newW*sizeof(RGBTRIPLE))%4)%4;
    RGBTRIPLE (*rot)[newW]=malloc(newH*sizeof(RGBTRIPLE[newW]));
    for(int i=0;i<*height;i++)
        for(int j=0;j<*width;j++)
            rot[newH-1-j][i]=(*image)[i][j];
    free(*image);
    *image=(RGBTRIPLE (*)[newW])rot;
    *height=newH; *width=newW; *padding=newP;
}

void resize(int *height, int *width, int *padding, RGBTRIPLE (**image)[*width],
            int newW, int newH, BITMAPFILEHEADER *bf, BITMAPINFOHEADER *bi){
    RGBTRIPLE (*resized)[newW]=malloc(newH*sizeof(RGBTRIPLE[newW]));
    for(int i=0;i<newH;i++)
        for(int j=0;j<newW;j++){
            int orig_i=i*(*height)/newH;
            int orig_j=j*(*width)/newW;
            resized[i][j]=(*image)[orig_i][orig_j];
        }
    free(*image);
    *image=(RGBTRIPLE (*)[newW])resized;
    *height=newH; *width=newW;
    *padding=(4-((newW*sizeof(RGBTRIPLE))%4))%4;
    bi->biWidth=*width; bi->biHeight=*height;
    bi->biSizeImage=(*height)*((*width)*sizeof(RGBTRIPLE)+*padding);
    bf->bfSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+bi->biSizeImage;
}
