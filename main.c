#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filters.h"

int main() {
    



    // taking input for read and write file
    char infile[100] = "./Images/desert.bmp";
    char outfile[100] = "./images/outsample1.bmp";


    // opening readfile
    FILE *readfile = fopen(infile, "rb");
    if (readfile == NULL) {
        printf("File %s not found\n", infile);
        return 1;
    }

    //opening writefile
    FILE *writefile = fopen(outfile, "wb");
    if (writefile == NULL) {
        printf("Couldn't create File %s\n", outfile);
        fclose(readfile);
        return 1;
    }

    // reading bmp image headers from readfile
    BITMAPFILEHEADER fileheader;
    fread(&fileheader, sizeof(BITMAPFILEHEADER), 1, readfile);
    BITMAPINFOHEADER infoheader;
    fread(&infoheader, sizeof(BITMAPINFOHEADER), 1, readfile);

    // dealing with standard 24 bit uncompressed bmp files only so ensuring file meets criteria
    if (fileheader.bfType != 0x4d42 || fileheader.bfOffBits != 54 || infoheader.biSize != 40 || infoheader.biBitCount != 24 || infoheader.biCompression != 0) {
        // 0x4d42 translates to MB, since most significant bit comes in the end, it is actually BM (BitMap), thus we are ensuring the file is a BitMap file
        // bfOffBits is the offsett (starting address) of the byte where pixel array starts, must be 54 = BITMAPFILEHEADER (14 bytes) + BITMAPINFOHEADER (40 bytes) for 24 bit uncompressed
        // Also ensure that infoheader size is 40
        // Bit count must be 24 
        printf("BITMAPFILEHEADER size: %lu\n", sizeof(BITMAPFILEHEADER));
        printf("BITMAPINFOHEADER size: %lu\n", sizeof(BITMAPINFOHEADER));
        printf("fileheader.bfType: %d\nfileheader.bfOffBits: %d\ninfoheader.biSize: %d\ninfoheader.biBitCount: %d\nbiCompression: %d\n", fileheader.bfType,fileheader.bfOffBits,infoheader.biSize, infoheader.biBitCount, infoheader.biCompression);
        printf("File is not a standard 24 bit uncompressed bmp file\n");
        fclose(writefile);
        fclose(readfile);
        return 1;
    }


    int width = infoheader.biWidth; 
    int height = abs(infoheader.biHeight); // can be negative if pixel array is stored top-down instead of the more common bottom-up (The pixel array starts from the bottom row of the image and moves upward to the top) approach.

    // allocating memory to create a 2D array of RGBTRIPLE, This declares pixelArray as a pointer to a 2D array where each row has width elements of type RGBTRIPLE.
    RGBTRIPLE(*pixelArray)[width] = calloc(height, width * sizeof(RGBTRIPLE));
    if (pixelArray == NULL) {
        printf("Memory allocation failed\n");
        fclose(writefile);
        fclose(readfile);
        return 1;
    }

    // The bits representing the bitmap pixels are packed in rows (also known as strides or scan lines). The size of each row is rounded up to a multiple of 4 bytes (a 32-bit DWORD) by padding which is stored at the end of each row
    // padding shall be ignored when loading data to pixelArray
    int padding = ( 4 - ((width*3) % 4) ) % 4;

    // Reading image pixel data from readfile 
    for (int i = 0; i < height; i++) {
        //reading width * 3 bytes
        fread(pixelArray[i], sizeof(RGBTRIPLE), width, readfile);

        //skip padding ( 4 - ((width*3) % 4) ) % 4 bytes
        fseek(readfile, padding, SEEK_CUR);
    }


    // Calling filters after user input!!!! - manipulating pixelArray

    printf("Select a filter to apply:\n");
    printf("1. Grayscale\n");
    printf("2. Sepia\n");
    printf("3. Reflect\n");
    printf("4. Blur\n");
    printf("5. Edges\n");
    printf("6. Adjust Brightness\n");
    printf("7. Adjust Contrast\n");
    printf("8. Invert Colors\n");
    printf("9. Pixelate\n");
    printf("10. Vignette\n");
    printf("11. Sharpen\n");
    printf("12. Gaussian Blur\n");
    printf("13. Emboss\n");
    printf("14. Rotate 90\n");
    printf("15. Rotate 180\n");
    printf("16. Rotate 270\n");
    printf("17. Add Border\n");
    int choice;
    do {
        printf("Enter your choice (1-17): ");
        scanf("%d", &choice);
        getchar();
        if (choice < 0 || choice > 17) {
            printf("Invalid choice, try again.\n");
        }
    } while(choice < 0 || choice > 17);

    // Declarations for user input
    int brightness;
    float contrast;
    int block_size;
    int max;
    RGBTRIPLE border_colour;
    switch(choice) {
        case 1:
            grayscale(height, width, pixelArray);
            break;
        case 2:
            sepia(height, width, pixelArray);
            break;
        case 3:
            reflect(height, width, pixelArray);
            break;
        case 4:
            blur(height, width, pixelArray);
            break;
        case 5:
            edges(height, width, pixelArray);
            break;
        case 6:
            do {
                printf("Enter brightness value to be added (-255 to 255 inclusive): ");
                scanf("%d", &brightness);
                if (brightness < -255 || brightness > 255) {
                    printf("Value out of range\n");
                }
            } while(brightness < -255 || brightness > 255);
            adjust_brightness(height, width, pixelArray, brightness);
            break;
        case 7:
            do {
                printf("Enter contrast factor value to be added (0.0 to 2.0 inclusive): ");
                scanf("%f", &contrast);
                if (contrast < 0.0 || contrast > 2.0) {
                    printf("Value out of range\n");
                }
            } while(contrast < -0.0 || contrast > 2.0);
            adjust_contrast(height, width, pixelArray, contrast);
            break;
        case 8:
            invert_colors(height, width, pixelArray);
            break;
        case 9:
            max = (height > width) ? width : height;
            do {
                printf("Enter pixelate value for square block_size (0 to %d): ", max);
                scanf("%d", &block_size);
                if (block_size < 0 || block_size > max) {
                    printf("Value out of range\n");
                }
            } while(block_size < 0 || block_size > max);
            pixelate(height, width, pixelArray, 10);
            break;
        case 10:
            vignette(height, width, pixelArray);
            break;
        case 11:
            sharpen(height, width, pixelArray);
            break;
        case 12:
            gaussian_blur(height, width, pixelArray);
            break;
        case 13:
            emboss(height, width, pixelArray);
            break;
        case 14:
            rotate_90(&infoheader.biHeight, &infoheader.biWidth, &padding, &pixelArray);
            break;
        case 15:
            rotate_180(height, width, pixelArray);
            break;
        case 16:
            rotate_270(&infoheader.biHeight, &infoheader.biWidth, &padding, &pixelArray);
            break;
        case 17:
            printf("A Pixel Color consists of Red Green & Blue values\n");
            do {
                printf("Enter Red color value for border_color (0 to 255 inclusive): ");
                scanf("%d", &border_colour.rgbtRed);
                if (border_colour.rgbtRed < 0 || border_colour.rgbtRed > 255) {
                    printf("Value out of range\n");
                }
            } while(border_colour.rgbtRed < 0 || border_colour.rgbtRed > 255);

            do {
                printf("Enter Green color value for border_color (0 to 255 inclusive): ");
                scanf("%d", &border_colour.rgbtGreen);
                if (border_colour.rgbtGreen < 0 || border_colour.rgbtGreen > 255) {
                    printf("Value out of range\n");
                }
            } while(border_colour.rgbtGreen < 0 || border_colour.rgbtGreen > 255);
            
            do {
                printf("Enter Blue color value for border_color (0 to 255 inclusive): ");
                scanf("%d", &border_colour.rgbtBlue);
                if (border_colour.rgbtBlue < 0 || border_colour.rgbtBlue > 255) {
                    printf("Value out of range\n");
                }
            } while(border_colour.rgbtBlue < 0 || border_colour.rgbtBlue > 255);

            add_border(height, width, pixelArray, 5, border_colour);
            break;
    }

    if (choice == 14 || choice == 16) {
        infoheader.biSizeImage = ((infoheader.biWidth * sizeof(RGBTRIPLE)) + padding) * infoheader.biHeight;
        fileheader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + infoheader.biSizeImage;
    }

    // Writing outfile's headers
    fwrite(&fileheader, sizeof(BITMAPFILEHEADER), 1, writefile);
    fwrite(&infoheader, sizeof(BITMAPINFOHEADER), 1, writefile);

    BYTE paddingValue = 0x00;
    // writing manipulated data to bitmap file
    for (int i = 0; i < height; i++) {
        fwrite(pixelArray[i], sizeof(RGBTRIPLE), width, writefile);
        // writing padding at the end of row
        fwrite(&paddingValue, sizeof(BYTE), padding, writefile);
    }

    printf("%s edited and saved as %s successfully\n", infile, outfile);

    fclose(writefile);
    fclose(readfile);
    free(pixelArray);
    return 0;
}
