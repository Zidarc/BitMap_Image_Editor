#include "filters.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void grayscale(int height, int width, RGBTRIPLE image[height][width]) {
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int sum = 0;
            sum += image[i][j].rgbtBlue;
            sum += image[i][j].rgbtGreen;
            sum += image[i][j].rgbtRed;

            image[i][j].rgbtBlue = round((double)sum/(double)3);
            image[i][j].rgbtGreen = round((double)sum/(double)3);
            image[i][j].rgbtRed = round((double)sum/(double)3);
        }
    }

    return;
}

void sepia(int height, int width, RGBTRIPLE image[height][width]) {
    for (int i = 0; i < height; i++) {
        int sepiaRed, sepiaGreen, sepiaBlue;
        for (int j = 0; j < width; j++) {

            sepiaRed = (0.393 * image[i][j].rgbtRed) + (0.769 * image[i][j].rgbtGreen) + (0.189 * image[i][j].rgbtBlue);
            if (sepiaRed > 255) {
                sepiaRed = 255;
            }

            sepiaGreen = (0.349 * image[i][j].rgbtRed) + (0.686 * image[i][j].rgbtGreen) + (0.168 * image[i][j].rgbtBlue);
            if (sepiaGreen > 255) {
               sepiaGreen = 255;
            }

            sepiaBlue = (0.272 * image[i][j].rgbtRed) + (0.534 * image[i][j].rgbtGreen) + (0.131 * image[i][j].rgbtBlue);
            if (sepiaBlue > 255) {
                sepiaBlue = 255;
            }

            image[i][j].rgbtRed = sepiaRed;
            image[i][j].rgbtGreen = sepiaGreen;
            image[i][j].rgbtBlue = sepiaBlue;
        }
    }
    return;
}

void reflect(int height, int width, RGBTRIPLE image[height][width]) {
    RGBTRIPLE temp;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width/2; j++) {
            temp = image[i][j];
            image[i][j] = image[i][width - 1 - j];
            image[i][width - 1 - j] = temp;
        }
    }
}

void blur(int height, int width, RGBTRIPLE image[height][width]) {

    RGBTRIPLE (*blurred)[width] = malloc(height * sizeof(RGBTRIPLE[width]));
    if (blurred == NULL) {
        printf("Memory allocation failed for edged array!\n");
        return;
    }
    for (int i = 0; i < height; i++) {

        for (int j = 0; j < width; j++) {
            int red = 0, green = 0, blue = 0;
            int count = 0;
            for (int k = i - 1; k < i + 2; k++) {
                
                for (int l = j - 1; l < j + 2; l++) {

                    if (k >= 0 && k < height && l >= 0 && l < width) {
                        count++;
                        red += image[k][l].rgbtRed;
                        green += image[k][l].rgbtGreen;
                        blue +=image[k][l].rgbtBlue;
                    }
                    
                }

            }
            blurred[i][j].rgbtRed = round((double)red/(double)count);
            blurred[i][j].rgbtGreen = round((double)green/(double)count);
            blurred[i][j].rgbtBlue = round((double)blue/(double)count);
        }

    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            image[i][j] = blurred[i][j];
        }
    }
    free(blurred);
    return;

}

void edges(int height, int width, RGBTRIPLE image[height][width]) {

    int Gx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    int Gy[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
    
    RGBTRIPLE (*edged)[width] = malloc(height * sizeof(RGBTRIPLE[width]));
    if (edged == NULL) {
        printf("Memory allocation failed for edged array!\n");
        return;
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            
            int RedxSum = 0, GreenxSum = 0, BluexSum = 0;
            int RedySum = 0, GreenySum = 0, BlueySum = 0;
            for (int k = i - 1; k < i + 2; k++) {
                for (int l = j - 1; l < j + 2; l++) {

                    if (k < 0 || k >= height || l < 0 || l >= width) {
                        continue;
                    }

                    int GRowindex = k - i + 1;
                    int GColindex = l - j + 1;

                    RedxSum += (Gx[GRowindex][GColindex] * image[k][l].rgbtRed);
                    GreenxSum += (Gx[GRowindex][GColindex] * image[k][l].rgbtGreen);
                    BluexSum += (Gx[GRowindex][GColindex] * image[k][l].rgbtBlue);

                    RedySum += (Gy[GRowindex][GColindex] * image[k][l].rgbtRed);
                    GreenySum += (Gy[GRowindex][GColindex] * image[k][l].rgbtGreen);
                    BlueySum += (Gy[GRowindex][GColindex] * image[k][l].rgbtBlue);
                }
            }
            edged[i][j].rgbtRed = fmin(round(sqrt(RedxSum * RedxSum + RedySum * RedySum)), 255);
            edged[i][j].rgbtGreen = fmin(round(sqrt(GreenxSum * GreenxSum + GreenySum * GreenySum)), 255);
            edged[i][j].rgbtBlue = fmin(round(sqrt(BluexSum * BluexSum + BlueySum * BlueySum)), 255);
        }
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            image[i][j] = edged[i][j];
        }
    }
    free(edged);
    return;

}

void adjust_brightness(int height, int width, RGBTRIPLE image[height][width], int brightness) {

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            image[i][j].rgbtRed = fmax(fmin(image[i][j].rgbtRed + brightness, 255), 0);
            image[i][j].rgbtGreen = fmax(fmin(image[i][j].rgbtGreen + brightness, 255), 0);
            image[i][j].rgbtBlue = fmax(fmin(image[i][j].rgbtBlue + brightness, 255), 0);
        }
    }
    return;
}

void adjust_contrast(int height, int width, RGBTRIPLE image[height][width], float contrast_factor) {

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            image[i][j].rgbtRed = fmax(fmin((image[i][j].rgbtRed - 128) * contrast_factor + 128, 255), 0);
            image[i][j].rgbtGreen = fmax(fmin((image[i][j].rgbtGreen - 128) * contrast_factor + 128, 255), 0);
            image[i][j].rgbtBlue = fmax(fmin((image[i][j].rgbtBlue - 128) * contrast_factor + 128, 255), 0);
        }
    }
    return;

}

void invert_colors(int height, int width, RGBTRIPLE image[height][width]) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            image[i][j].rgbtRed = 255 - image[i][j].rgbtRed;
            image[i][j].rgbtGreen = 255 - image[i][j].rgbtGreen;
            image[i][j].rgbtBlue = 255 - image[i][j].rgbtBlue;
        }
    }
    return;
}

void pixelate(int height, int width, RGBTRIPLE image[height][width], int block_size) {

    for (int i = 0; i < height; i += block_size) {

        for (int j = 0; j < width; j += block_size) {
            int redSum = 0, greenSum = 0, blueSum = 0, block_pixels = 0;

            for (int row = i; row < fmin(i + block_size, height); row++) {
                for (int col = j; col < fmin(j + block_size, width); col++) {
                    redSum += image[row][col].rgbtRed;
                    greenSum += image[row][col].rgbtGreen;
                    blueSum += image[row][col].rgbtBlue;
                    block_pixels++;
                }
            }

            int avgRed = fmax(fmin(round((double)redSum / block_pixels), 255), 0);
            int avgGreen = fmax(fmin(round((double)greenSum / block_pixels), 255), 0);
            int avgBlue = fmax(fmin(round((double)blueSum / block_pixels), 255), 0);

            for (int row = i; row < fmin(i + block_size, height); row++) {
                for (int col = j; col < fmin(j + block_size, width); col++) {
                    image[row][col].rgbtRed = avgRed;
                    image[row][col].rgbtGreen = avgGreen;
                    image[row][col].rgbtBlue = avgBlue;
                }
            }
        }

    }

    return;
}


void vignette(int height, int width, RGBTRIPLE image[height][width]) {
    
    int centerX = width/2;
    int centerY = height/2;

    double max_distance = sqrt(pow(centerX, 2) + pow(centerY, 2));

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {

            double dist = sqrt(pow(centerX - j, 2) + pow(centerY - i, 2));
            double factor =1.0 -  dist/max_distance;
            image[i][j].rgbtRed = fmax(fmin(image[i][j].rgbtRed * factor, 255),0);
            image[i][j].rgbtGreen = fmax(fmin(image[i][j].rgbtGreen * factor, 255),0);
            image[i][j].rgbtBlue = fmax(fmin(image[i][j].rgbtBlue * factor, 255),0);

        }
    }

    return;

}

void sharpen(int height, int width, RGBTRIPLE image[height][width]) {

    RGBTRIPLE (*sharpened)[width] = malloc(height * sizeof(RGBTRIPLE[width]));
    if (sharpened == NULL) {
        printf("Memory allocation failed for edged array!\n");
        return;
    }
    int kernel[3][3] = 
    {   {0, -1, 0}, 
        {-1, 5, -1}, 
        {0, -1, 0}
    };
    for (int i = 0; i < height; i++) {

        for (int j = 0; j < width; j++) {
            int Red = 0, Blue = 0, Green = 0;
            for (int k = i - 1; k < i + 2; k++) {
                for (int l = j - 1; l < j + 2; l++) {
                    
                    if (k < 0 || k >= height || l < 0 || l >= width) {
                        continue;
                    }

                    int row = k - i + 1;
                    int col = l - j + 1;

                    Red += (kernel[row][col] * image[k][l].rgbtRed);
                    Green += (kernel[row][col] * image[k][l].rgbtGreen);
                    Blue += (kernel[row][col] * image[k][l].rgbtBlue);
                    
                }
            }
            sharpened[i][j].rgbtRed = fmax(fmin(Red, 255), 0);
            sharpened[i][j].rgbtGreen = fmax(fmin(Green, 255), 0);
            sharpened[i][j].rgbtBlue = fmax(fmin(Blue, 255), 0);
        }

    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            image[i][j] = sharpened[i][j];
        }
    }
    free(sharpened);
}

void gaussian_blur(int height, int width, RGBTRIPLE image[height][width]) {

    RGBTRIPLE (*blurred)[width] = malloc(height * sizeof(RGBTRIPLE[width]));
    if (blurred == NULL) {
        printf("Memory allocation failed for edged array!\n");
        return;
    }
    float kernel[5][5] = {
        {1.0 / 256, 4.0 / 256, 6.0 / 256, 4.0 / 256, 1.0 / 256},
        {4.0 / 256, 16.0 / 256, 24.0 / 256, 16.0 / 256, 4.0 / 256},
        {6.0 / 256, 24.0 / 256, 36.0 / 256, 24.0 / 256, 6.0 / 256},
        {4.0 / 256, 16.0 / 256, 24.0 / 256, 16.0 / 256, 4.0 / 256},
        {1.0 / 256, 4.0 / 256, 6.0 / 256, 4.0 / 256, 1.0 / 256}
    };

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            float Red = 0.0, Green = 0.0, Blue = 0.0;
            for (int k = i - 2; k < i + 3; k++) {
                for (int l = j - 2; l < j + 3; l++) {
                    
                    if (k < 0 || k >= height || l < 0 || l >= width) {
                        continue;
                    }

                    int row = k - i + 2;
                    int col = l - j + 2;

                    Red += (kernel[row][col] * image[k][l].rgbtRed);
                    Green += (kernel[row][col] * image[k][l].rgbtGreen);
                    Blue += (kernel[row][col] * image[k][l].rgbtBlue);
                    
                }
            }
            blurred[i][j].rgbtRed = fmax(fmin(round(Red), 255), 0);
            blurred[i][j].rgbtGreen = fmax(fmin(round(Green), 255), 0);
            blurred[i][j].rgbtBlue = fmax(fmin(round(Blue), 255), 0);
        }
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            image[i][j] = blurred[i][j];
        }
    }
    free(blurred);
}

void emboss(int height, int width, RGBTRIPLE image[height][width]) {
    
    RGBTRIPLE (*embossed)[width] = malloc(height * sizeof(RGBTRIPLE[width]));
    if (embossed == NULL) {
        printf("Memory allocation failed for edged array!\n");
        return;
    }
    int kernel[3][3] = {
        {-2, -1,  0},
        {-1,  1,  1},
        { 0,  1,  2}
    };

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            float Red = 0.0, Green = 0.0, Blue = 0.0;
            
            for (int k = i - 1; k < i + 2; k++) {
                for (int l = j - 1; l < j + 2; l++) {
                    if (k < 0 || k >= height || l < 0 || l >= width) {
                        continue;
                    }

                    int row = k - i + 1;
                    int col = l - j + 1;

                    Red += (kernel[row][col] * image[k][l].rgbtRed);
                    Green += (kernel[row][col] * image[k][l].rgbtGreen);
                    Blue += (kernel[row][col] * image[k][l].rgbtBlue);
                    
                }
            }
            embossed[i][j].rgbtRed = fmax(fmin(round(Red), 255), 0);
            embossed[i][j].rgbtGreen = fmax(fmin(round(Green), 255), 0);
            embossed[i][j].rgbtBlue = fmax(fmin(round(Blue), 255), 0);
        }
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            image[i][j] = embossed[i][j];
        }
    }
    free(embossed);
    return;
}

void rotate_90(int *height, int *width, int *padding, RGBTRIPLE (**image)[*width]) {
    int newHeight = *width;
    int newWidth = *height;
    int newPadding = (4 - (newWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    RGBTRIPLE (*rotated)[newWidth] = malloc(newHeight * sizeof(RGBTRIPLE[newWidth]));
    if (rotated == NULL) {
        printf("Memory allocation failed!\n");
        return;
    }

    for (int i = 0; i < *height; i++) {
        for (int j = 0; j < *width; j++) {
            rotated[j][newWidth - 1 - i] = (*image)[i][j];
        }
    }

    free(*image);
    *image = (RGBTRIPLE (*)[newWidth])rotated;

    *height = newHeight;
    *width = newWidth;
    *padding = newPadding;
}

void rotate_180(int height, int width, RGBTRIPLE image[height][width]) {
    RGBTRIPLE temp;
    for (int i = 0; i < height / 2; i++) {
        for (int j = 0; j < width; j++) {
            temp = image[i][j];
            image[i][j] = image[height - 1 - i][width - 1 - j];
            image[height - 1 - i][width - 1 - j] = temp;
        }
    }

    if (height % 2 != 0) { // handle middle row for odd height
        int middleRow = height / 2;
        for (int j = 0; j < width / 2; j++) {
            temp = image[middleRow][j];
            image[middleRow][j] = image[middleRow][width - 1 - j];
            image[middleRow][width - 1 - j] = temp;
        }
    }
}

void rotate_270(int *height, int *width, int *padding, RGBTRIPLE (**image)[*width]) {
    int newHeight = *width;
    int newWidth = *height;
    int newPadding = (4 - (newWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    RGBTRIPLE (*rotated)[newWidth] = malloc(newHeight * sizeof(RGBTRIPLE[newWidth]));
    if (rotated == NULL) {
        printf("Memory allocation failed!\n");
        return;
    }

    for (int i = 0; i < *height; i++) {
        for (int j = 0; j < *width; j++) {
            rotated[newHeight - 1 - j][i] = (*image)[i][j];
        }
    }

    free(*image);
    *image = (RGBTRIPLE (*)[newWidth])rotated;

    *height = newHeight;
    *width = newWidth;
    *padding = newPadding;
}


void add_border(int height, int width, RGBTRIPLE image[height][width], int border_width, RGBTRIPLE border_color) {

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < border_width; j++) {
            image[i][j] = border_color;
        }
        for (int j = width - border_width ; j < width; j++) {
            image[i][j] = border_color;
        }
    }

    for (int j = border_width; j < width - border_width; j++) {
        for (int i = 0; i < border_width; i++) {
            image[i][j] = border_color;
        }
        for (int i = height - border_width; i < height; i++) {
            image[i][j] = border_color;
        }
    }

    return;
}
void resize(int *height, int *width, int *padding, RGBTRIPLE (**image)[*width], int new_width, int new_height, BITMAPFILEHEADER *fileheader, BITMAPINFOHEADER *infoheader)
{
    if (new_width <= 0 || new_height <= 0 || new_width > *width || new_height > *height) {
        printf("Invalid dimensions for resizing.\n");
        return;
    }

    // Allocate new pixel array for resized image
    RGBTRIPLE (*resized)[new_width] = malloc(new_height * sizeof(RGBTRIPLE[new_width]));
    if (!resized) {
        printf("Memory allocation failed for resized image.\n");
        return;
    }

    // Nearest-neighbor scaling: pick the nearest pixel from original
    for (int i = 0; i < new_height; i++) {
        for (int j = 0; j < new_width; j++) {
            int orig_i = i * (*height) / new_height;
            int orig_j = j * (*width) / new_width;
            resized[i][j] = (*image)[orig_i][orig_j];
        }
    }

    // Free old pixel array
    free(*image);
    *image = (RGBTRIPLE (*)[new_width])resized;

    // Update width and height
    *width = new_width;
    *height = new_height;

    // Recalculate padding
    *padding = (4 - ((*width * sizeof(RGBTRIPLE)) % 4)) % 4;

    // Update infoheader and fileheader
    infoheader->biWidth = *width;
    infoheader->biHeight = *height;
    infoheader->biSizeImage = ((*width * sizeof(RGBTRIPLE)) + *padding) * (*height);
    fileheader->bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + infoheader->biSizeImage;

    printf("Image resized to %dx%d successfully.\n", new_width, new_height);
}