#include "bmp.h"

//1. Convert image to grayscale
void grayscale(int height, int width, RGBTRIPLE image[height][width]);

//2. Convert image to sepia
void sepia(int height, int width, RGBTRIPLE image[height][width]);

//3. Reflect image horizontally
void reflect(int height, int width, RGBTRIPLE image[height][width]);

//4. Blur image
void blur(int height, int width, RGBTRIPLE image[height][width]);

//5. Detect edges (thinking of removing this)
void edges(int height, int width, RGBTRIPLE image[height][width]);

//6. Adjust brightness of the image
void adjust_brightness(int height, int width, RGBTRIPLE image[height][width], int brightness);

//7. Adjust contrast of the image
void adjust_contrast(int height, int width, RGBTRIPLE image[height][width], float contrast_factor);

//8. Invert the colors of the image
void invert_colors(int height, int width, RGBTRIPLE image[height][width]);

//9. Apply pixelation effect to the image
void pixelate(int height, int width, RGBTRIPLE image[height][width], int block_size);

//10. Apply vignette effect
void vignette(int height, int width, RGBTRIPLE image[height][width]);

//11. Sharpen the image
void sharpen(int height, int width, RGBTRIPLE image[height][width]);

//12. Apply Gaussian blur to the image
void gaussian_blur(int height, int width, RGBTRIPLE image[height][width]);

//13. Emboss the image
void emboss(int height, int width, RGBTRIPLE image[height][width]);

//14. Rotate image by 90 degrees
void rotate_90(int *height, int *width, int *padding, RGBTRIPLE (**image)[*width]);

//15. Rotate image by 180 degrees
void rotate_180(int height, int width, RGBTRIPLE image[height][width]);

//16. Rotate image by 270 degrees
void rotate_270(int *height, int *width, int *padding, RGBTRIPLE (**image)[*width]);

//17. Add border to image
void add_border(int height, int width, RGBTRIPLE image[height][width], int border_width, RGBTRIPLE border_color);

void resize(int *height, int *width, int *padding, RGBTRIPLE (**image)[*width], int new_width, int new_height, BITMAPFILEHEADER *fileheader, BITMAPINFOHEADER *infoheader);
