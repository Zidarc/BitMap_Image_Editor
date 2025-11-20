// gui_main_integrated.c
// Integrated GUI + existing login + BMP load, apply filters, save to output/<username>_<filter>.bmp
// No preview (Option A). Login logic untouched (calls signup() and login()).

#include "raylib.h"
#include <stdio.h>
#include <bmp.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#if defined(_WIN32)
#include <direct.h> // _mkdir
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h> // mkdir
#define MKDIR(path) mkdir(path, 0755)
#endif

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "filters.h"   // your filter declarations
#include "bmp.h"       // BITMAPFILEHEADER, BITMAPINFOHEADER, RGBTRIPLE, BYTE

// Keep your login prototypes exactly as before
int signup(const char *username, const char *password);
int login(const char *username, const char *password);

typedef enum GameScreen { LOGIN_SCREEN, SIGNUP_SCREEN, MAIN_SCREEN } GameScreen;

// GUI layout constants (kept same as your original)
static int screenWidth = 1300;
static int screenHeight = 700;

static Rectangle leftPanel   = {10, 40, 240, 640};     // HISTORY
static Rectangle centerPanel = {260, 40, 760, 640};    // FILTERS + TEMPLATES
static Rectangle rightPanel  = {1040, 40, 250, 640};   // LOAD IMAGE

// Filter list (expanded to match filters you have)
static const char *filterOptions[] = {
    "Resize", "Rotate 90°", "Rotate 180°", "Flip Horizontal",
    "Flip Vertical", "Grayscale", "Sepia", "Invert",
    "Blur", "Edges", "Brightness", "Contrast", "Pixelate",
    "Vignette", "Sharpen", "Gaussian Blur", "Emboss",
    "Rotate 270", "Add Border"
};
static int filterCount = sizeof(filterOptions)/sizeof(filterOptions[0]);
static int selectedFilter = -1;

// Template list (unchanged)
static const char *templateOptions[] = {
    "Facebook","Instagram","YouTube","Twitter",
    "TikTok","Snapchat","LinkedIn"
};
static int templateCount = sizeof(templateOptions)/sizeof(templateOptions[0]);
static int selectedTemplate = -1;

// Load image path input (user types e.g. Images/desert.bmp)
static char pathInput[512] = "Images/desert.bmp";

// Output folder
static const char *outputDir = "output";

// In-memory BMP representation
static void *rawPixels = NULL; // contiguous block: height * width * sizeof(RGBTRIPLE)
static int imgWidth = 0;
static int imgHeight = 0;
static int imgPadding = 0;
static BITMAPFILEHEADER fileheader;
static BITMAPINFOHEADER infoheader;
static int imageLoaded = 0; // 0 = none, 1 = loaded

// Logged-in user (from your login flow)
static char loggedInUser[64] = "";

// GUI message (status / errors)
static char guiMessage[512] = "";

// Tiny helper to set GUI message (formatted)
static void setGuiMessage(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(guiMessage, sizeof(guiMessage), fmt, ap);
    va_end(ap);
}

// Helper: free pixel memory
static void free_pixels(void)
{
    if (rawPixels) {
        free(rawPixels);
        rawPixels = NULL;
    }
    imgWidth = imgHeight = imgPadding = 0;
    imageLoaded = 0;
}

// Load .bmp to rawPixels. Returns 1 on success, 0 on failure.
// Accepts path relative to program working directory.
static int load_bmp(const char *path)
{
    if (!path || strlen(path) == 0) {
        setGuiMessage("Load path empty.");
        return 0;
    }

    FILE *fp = fopen(path, "rb");
    if (!fp) {
        setGuiMessage("Failed to open file: %s", path);
        return 0;
    }

    // Read headers
    if (fread(&fileheader, sizeof(BITMAPFILEHEADER), 1, fp) != 1 ||
        fread(&infoheader, sizeof(BITMAPINFOHEADER), 1, fp) != 1) {
        fclose(fp);
        setGuiMessage("Failed to read BMP headers: %s", path);
        return 0;
    }

    // Validate standard 24-bit uncompressed BMP
    if (fileheader.bfType != 0x4D42 || fileheader.bfOffBits != 54 ||
        infoheader.biSize != 40 || infoheader.biBitCount != 24 || infoheader.biCompression != 0) {
        fclose(fp);
        setGuiMessage("File is not a standard 24-bit uncompressed BMP: %s", path);
        return 0;
    }

    imgWidth = infoheader.biWidth;
    imgHeight = abs(infoheader.biHeight);
    imgPadding = (4 - ((imgWidth * 3) % 4)) % 4;

    // allocate contiguous block
    size_t totalBytes = (size_t)imgWidth * (size_t)imgHeight * sizeof(RGBTRIPLE);
    void *buf = malloc(totalBytes);
    if (!buf) {
        fclose(fp);
        setGuiMessage("Memory allocation failed (%zu bytes)", totalBytes);
        return 0;
    }

    // Seek to pixel data
    fseek(fp, fileheader.bfOffBits, SEEK_SET);

    // Read each row (BMP stored bottom-up usually; we will store in file order)
    for (int row = 0; row < imgHeight; row++) {
        RGBTRIPLE *rowPtr = (RGBTRIPLE *)((unsigned char *)buf + (size_t)row * (size_t)imgWidth * sizeof(RGBTRIPLE));
        size_t got = fread(rowPtr, sizeof(RGBTRIPLE), imgWidth, fp);
        if (got != (size_t)imgWidth) {
            free(buf);
            fclose(fp);
            setGuiMessage("Failed reading pixel row %d from %s", row, path);
            return 0;
        }
        if (imgPadding) fseek(fp, imgPadding, SEEK_CUR);
    }

    fclose(fp);

    // Replace any old pixels
    free_pixels();
    rawPixels = buf;
    imageLoaded = 1;
    setGuiMessage("Loaded: %s  (w:%d h:%d)", path, imgWidth, imgHeight);
    return 1;
}

// Save rawPixels to path. Returns 1 on success.
static int save_bmp(const char *path)
{
    if (!imageLoaded || !rawPixels) {
        setGuiMessage("No image loaded to save.");
        return 0;
    }
    if (!path || strlen(path) == 0) {
        setGuiMessage("Save path empty.");
        return 0;
    }

    // Prepare headers for output (update sizes)
    BITMAPFILEHEADER outFile = fileheader;
    BITMAPINFOHEADER outInfo = infoheader;

    outInfo.biWidth = imgWidth;
    // preserve original sign of biHeight (top-down vs bottom-up)
    outInfo.biHeight = (infoheader.biHeight < 0) ? -imgHeight : imgHeight;

    int padding = (4 - ((imgWidth * 3) % 4)) % 4;
    outInfo.biSizeImage = ((imgWidth * sizeof(RGBTRIPLE)) + padding) * imgHeight;
    outFile.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + outInfo.biSizeImage;

    FILE *wf = fopen(path, "wb");
    if (!wf) {
        setGuiMessage("Failed to create output file: %s", path);
        return 0;
    }

    if (fwrite(&outFile, sizeof(BITMAPFILEHEADER), 1, wf) != 1 ||
        fwrite(&outInfo, sizeof(BITMAPINFOHEADER), 1, wf) != 1) {
        fclose(wf);
        setGuiMessage("Failed to write headers to %s", path);
        return 0;
    }

    BYTE padVal = 0x00;
    for (int row = 0; row < imgHeight; row++) {
        RGBTRIPLE *rowPtr = (RGBTRIPLE *)((unsigned char *)rawPixels + (size_t)row * (size_t)imgWidth * sizeof(RGBTRIPLE));
        if (fwrite(rowPtr, sizeof(RGBTRIPLE), imgWidth, wf) != (size_t)imgWidth) {
            fclose(wf);
            setGuiMessage("Failed writing pixel data to %s", path);
            return 0;
        }
        if (padding) fwrite(&padVal, sizeof(BYTE), padding, wf);
    }

    fclose(wf);
    setGuiMessage("Saved: %s", path);
    return 1;
}

// Build output path of form: ./output/<username>_<filtername>.bmp
static void build_output_path(char *outBuf, size_t outBufSize, const char *username, const char *filtername)
{
    // ensure output dir exists
    MKDIR(outputDir);
    snprintf(outBuf, outBufSize, "%s/%s_%s.bmp", outputDir, username ? username : "user", filtername ? filtername : "out");
}

// Map filter index to name for filename
static const char *filter_name_for_index(int idx)
{
    if (idx >= 0 && idx < filterCount) return filterOptions[idx];
    return "filter";
}

// Apply selected filter (uses your filters.c functions). After apply, auto-save to output/<username>_<filter>.bmp
static void apply_filter_and_save(int idx)
{
    if (!imageLoaded || !rawPixels) {
        setGuiMessage("No image loaded. Use Load button to load a BMP first.");
        return;
    }
    if (loggedInUser[0] == '\0') {
        setGuiMessage("No logged in user. Login required.");
        return;
    }

    // We need to create a typed pointer for passing to functions:
    // RGBTRIPLE (*image2)[imgWidth] = (RGBTRIPLE (*)[imgWidth]) rawPixels;
    // For functions that can change pointer or dimensions (rotate_90/270/resize) they expect RGBTRIPLE (**image)[*width]
    // We'll follow an approach: create a typed pointer variable, pass &typedPtr to functions that may reassign,
    // then update rawPixels and imgWidth/imgHeight if changed.

    // Typed pointer (current width type)
    RGBTRIPLE (*typedPtr)[imgWidth] = (RGBTRIPLE (*)[imgWidth]) rawPixels;

    // default values for parameterized filters
    const int default_brightness = 50;
    const float default_contrast = 1.2f;
    const int default_pixelate_block = 10;
    const int default_border_width = 5;
    RGBTRIPLE default_border_color = {.rgbtRed = 255, .rgbtGreen = 255, .rgbtBlue = 255};
    int new_w, new_h;

    switch(idx) {
        case 0: // Resize (example: half)
            new_w = imgWidth / 2; if (new_w < 1) new_w = 1;
            new_h = imgHeight / 2; if (new_h < 1) new_h = 1;
            // call resize expecting pointer-to-typed-pointer
            resize(&imgHeight, &imgWidth, &imgPadding, (RGBTRIPLE (**)[imgWidth]) &typedPtr, new_w, new_h, &fileheader, &infoheader);
            // update rawPixels to new pointer (typedPtr might have changed)
            rawPixels = (void *)typedPtr;
            setGuiMessage("Applied Resize to %dx%d", imgWidth, imgHeight);
            break;

        case 1: // Rotate 90
            rotate_90(&imgHeight, &imgWidth, &imgPadding, (RGBTRIPLE (**)[imgWidth]) &typedPtr);
            rawPixels = (void *)typedPtr;
            setGuiMessage("Applied Rotate 90 (w:%d h:%d)", imgWidth, imgHeight);
            break;

        case 2: // Rotate 180
            rotate_180(imgHeight, imgWidth, (RGBTRIPLE (*)[imgWidth]) rawPixels);
            setGuiMessage("Applied Rotate 180");
            break;

        case 3: // Flip Horizontal
            reflect(imgHeight, imgWidth, (RGBTRIPLE (*)[imgWidth]) rawPixels);
            setGuiMessage("Applied Flip Horizontal");
            break;

        case 4: // Flip Vertical (we'll use rotate_180 for this)
            rotate_180(imgHeight, imgWidth, (RGBTRIPLE (*)[imgWidth]) rawPixels);
            setGuiMessage("Applied Flip Vertical (via rotate_180)");
            break;

        case 5: // Grayscale
            grayscale(imgHeight, imgWidth, (RGBTRIPLE (*)[imgWidth]) rawPixels);
            setGuiMessage("Applied Grayscale");
            break;

        case 6: // Sepia
            sepia(imgHeight, imgWidth, (RGBTRIPLE (*)[imgWidth]) rawPixels);
            setGuiMessage("Applied Sepia");
            break;

        case 7: // Invert
            invert_colors(imgHeight, imgWidth, (RGBTRIPLE (*)[imgWidth]) rawPixels);
            setGuiMessage("Applied Invert");
            break;

        case 8: // Blur
            blur(imgHeight, imgWidth, (RGBTRIPLE (*)[imgWidth]) rawPixels);
            setGuiMessage("Applied Blur");
            break;

        case 9: // Edges
            edges(imgHeight, imgWidth, (RGBTRIPLE (*)[imgWidth]) rawPixels);
            setGuiMessage("Applied Edges");
            break;

        case 10: // Brightness
            adjust_brightness(imgHeight, imgWidth, (RGBTRIPLE (*)[imgWidth]) rawPixels, default_brightness);
            setGuiMessage("Adjusted Brightness (%+d)", default_brightness);
            break;

        case 11: // Contrast
            adjust_contrast(imgHeight, imgWidth, (RGBTRIPLE (*)[imgWidth]) rawPixels, default_contrast);
            setGuiMessage("Adjusted Contrast (%.2f)", default_contrast);
            break;

        case 12: // Pixelate
            pixelate(imgHeight, imgWidth, (RGBTRIPLE (*)[imgWidth]) rawPixels, default_pixelate_block);
            setGuiMessage("Applied Pixelate (block %d)", default_pixelate_block);
            break;

        case 13: // Vignette
            vignette(imgHeight, imgWidth, (RGBTRIPLE (*)[imgWidth]) rawPixels);
            setGuiMessage("Applied Vignette");
            break;

        case 14: // Sharpen
            sharpen(imgHeight, imgWidth, (RGBTRIPLE (*)[imgWidth]) rawPixels);
            setGuiMessage("Applied Sharpen");
            break;

        case 15: // Gaussian Blur
            gaussian_blur(imgHeight, imgWidth, (RGBTRIPLE (*)[imgWidth]) rawPixels);
            setGuiMessage("Applied Gaussian Blur");
            break;

        case 16: // Emboss
            emboss(imgHeight, imgWidth, (RGBTRIPLE (*)[imgWidth]) rawPixels);
            setGuiMessage("Applied Emboss");
            break;

        case 17: // Rotate 270
            rotate_270(&imgHeight, &imgWidth, &imgPadding, (RGBTRIPLE (**)[imgWidth]) &typedPtr);
            rawPixels = (void *)typedPtr;
            setGuiMessage("Applied Rotate 270 (w:%d h:%d)", imgWidth, imgHeight);
            break;

        case 18: // Add Border (default white, width 5)
            add_border(imgHeight, imgWidth, (RGBTRIPLE (*)[imgWidth]) rawPixels, default_border_width, default_border_color);
            setGuiMessage("Added border (%d px)", default_border_width);
            break;

        default:
            setGuiMessage("Unknown filter index: %d", idx);
            break;
    }

    // After apply, save automatically to output/<username>_<filter>.bmp
    char outpath[1024];
    // sanitize filter name for filename: replace spaces and degree char with underscore
    const char *rawFilterName = filter_name_for_index(idx);
    char safeFilter[256]; memset(safeFilter, 0, sizeof(safeFilter));
    size_t ri = 0;
    for (size_t i = 0; i < strlen(rawFilterName) && ri < sizeof(safeFilter)-1; i++) {
        char c = rawFilterName[i];
        if (c == ' ' || c == '°' || c == '/' || c == '\\') safeFilter[ri++] = '_';
        else safeFilter[ri++] = c;
    }
    safeFilter[ri] = '\0';

    build_output_path(outpath, sizeof(outpath), loggedInUser, safeFilter);

    if (!save_bmp(outpath)) {
        // save_bmp sets message
        return;
    }
    // saved: save_bmp sets success message, but also we can add location
    // setGuiMessage already set by save_bmp
}

// ------------------ MAIN (GUI + login) ------------------
int main(void)
{
    InitWindow(screenWidth, screenHeight, "BMT Image Project");
    SetTargetFPS(60);

    GameScreen currentScreen = LOGIN_SCREEN;

    // Login/signup variables (kept exactly as your original)
    char username[32] = "";
    char password[32] = "";
    char confirmPassword[32] = "";
    bool showPassword = false;

    // Textbox focus tracking
    int textBoxActive = 0; // 0=username, 1=password, 2=confirmPassword (signup)

    // Ensure output directory exists at startup
    MKDIR(outputDir);

    while (!WindowShouldClose()) {
        // Switch textbox with TAB
        if (IsKeyPressed(KEY_TAB)) {
            textBoxActive++;
            if (currentScreen == LOGIN_SCREEN) {
                if (textBoxActive > 1) textBoxActive = 0; // only username & password
            } else if (currentScreen == SIGNUP_SCREEN) {
                if (textBoxActive > 2) textBoxActive = 0; // username, password, confirmPassword
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch (currentScreen) {

            // ---------------- LOGIN SCREEN ----------------
            case LOGIN_SCREEN:
                DrawText("Login", 350, 50, 30, DARKGREEN);
                DrawText("Username:", 200, 150, 20, BLACK);
                if (GuiTextBox((Rectangle){320, 145, 250, 30}, username, 32, textBoxActive == 0))
                    textBoxActive = 0; // click focus

                DrawText("Password:", 200, 200, 20, BLACK);
                if (GuiTextBox((Rectangle){320, 195, 250, 30}, password, 32, textBoxActive == 1))
                    textBoxActive = 1; // click focus

                if (GuiButton((Rectangle){320, 250, 100, 40}, "Login")) {
                    if (login(username, password)) {
                        // copy username into loggedInUser (keep your behavior)
                        strncpy(loggedInUser, username, sizeof(loggedInUser)-1);
                        loggedInUser[sizeof(loggedInUser)-1] = '\0';
                        currentScreen = MAIN_SCREEN;
                        setGuiMessage("Logged in as %s", loggedInUser);
                    } else {
                        setGuiMessage("Login failed for %s", username);
                    }
                }

                if (GuiButton((Rectangle){430, 250, 150, 40}, "Sign Up")) {
                    strcpy(username, "");
                    strcpy(password, "");
                    strcpy(confirmPassword, "");
                    textBoxActive = 0;
                    currentScreen = SIGNUP_SCREEN;
                }
                break;

            // ---------------- SIGNUP SCREEN ----------------
            case SIGNUP_SCREEN:
                DrawText("Sign Up", 330, 50, 30, DARKPURPLE);
                DrawText("Username:", 200, 120, 20, BLACK);
                if (GuiTextBox((Rectangle){350, 115, 250, 30}, username, 32, textBoxActive == 0))
                    textBoxActive = 0; // click focus

                DrawText("Password:", 200, 170, 20, BLACK);
                if (GuiTextBox((Rectangle){350, 165, 250, 30}, password, 32, textBoxActive == 1))
                    textBoxActive = 1; // click focus

                DrawText("Confirm Password:", 200, 220, 20, BLACK);
                if (GuiTextBox((Rectangle){350, 215, 250, 30}, confirmPassword, 32, textBoxActive == 2))
                    textBoxActive = 2; // click focus

                if (GuiButton((Rectangle){350, 320, 100, 40}, "Sign Up")) {
                    if (strcmp(password, confirmPassword) != 0) {
                        // Keep your existing visual behavior: it draws text inline, but we will also set GUI message
                        setGuiMessage("Passwords do not match!");
                    } else if (signup(username, password)) {
                        strcpy(username, "");
                        strcpy(password, "");
                        strcpy(confirmPassword, "");
                        textBoxActive = 0;
                        currentScreen = LOGIN_SCREEN;
                        setGuiMessage("Sign up successful. Please log in.");
                    } else {
                        setGuiMessage("Sign up failed! Username may exist.");
                    }
                }

                if (GuiButton((Rectangle){460, 320, 100, 40}, "Back")) {
                    textBoxActive = 0;
                    currentScreen = LOGIN_SCREEN;
                }
                break;

            // ---------------- MAIN GUI ----------------
            case MAIN_SCREEN:
            {
                // Top bar
                DrawRectangle(0, 0, screenWidth, 36, LIGHTGRAY);
                DrawText("BMT Image Project", 12, 8, 14, DARKBLUE);
                DrawText(loggedInUser, 650, 8, 14, BLACK);

                // ---------------- LEFT PANEL ----------------
                DrawRectangleRec(leftPanel, Fade(DARKGRAY, 0.08f));
                DrawText("History", leftPanel.x + 8, leftPanel.y + 8, 20, DARKGRAY);
                DrawText("History disabled", leftPanel.x + 12, leftPanel.y + 50, 14, GRAY);

                // ---------------- RIGHT PANEL ----------------
                DrawRectangleRec(rightPanel, Fade(DARKGRAY, 0.08f));
                DrawText("Load Image", rightPanel.x + 8, rightPanel.y + 8, 20, DARKGRAY);
                DrawText("Path:", rightPanel.x + 8, rightPanel.y + 50, 14, DARKGRAY);
                // User types path here (e.g. Images/courtyard.bmp)
                GuiTextBox((Rectangle){ rightPanel.x + 8, rightPanel.y + 70, rightPanel.width - 16, 30 }, pathInput, sizeof(pathInput), true);
                if (GuiButton((Rectangle){ rightPanel.x + 8, rightPanel.y + 110, rightPanel.width - 16, 30 }, "Load")) {
                    // Attempt to load the BMP
                    if (!load_bmp(pathInput)) {
                        // load_bmp sets gui message on failure
                    }
                }

                // ---------------- CENTER PANEL ----------------
                DrawRectangleRec(centerPanel, Fade(DARKGRAY, 0.02f));
                Rectangle filtersPanel = { centerPanel.x + 10, centerPanel.y + 10, (centerPanel.width-30)/2, centerPanel.height-20 };
                Rectangle templatesPanel = { centerPanel.x + 20 + (centerPanel.width-30)/2, centerPanel.y + 10, (centerPanel.width-30)/2, centerPanel.height-20 };

                // --- Filters ---
                DrawRectangleRec(filtersPanel, Fade(DARKGRAY, 0.02f));
                DrawText("Filters", filtersPanel.x, filtersPanel.y, 24, DARKBLUE); // bigger
                Rectangle filterView = { filtersPanel.x, filtersPanel.y + 40, filtersPanel.width, filtersPanel.height - 50 };
                Rectangle filterContent = { 0, 0, filterView.width - 20, filterCount * 40 }; // bigger items
                static Vector2 filterScroll = {0};
                GuiScrollPanel(filterView, NULL, filterContent, &filterScroll, NULL);
                BeginScissorMode(filterView.x, filterView.y, filterView.width, filterView.height);
                for (int i = 0; i < filterCount; i++) {
                    Rectangle item = { filterView.x, filterView.y + i*40 + (int)filterScroll.y, filterView.width, 40 };

                    if (GuiButton(item, filterOptions[i])) {
                        selectedFilter = i;
                        selectedTemplate = -1; // deselect template
                        // WHEN FILTER BUTTON CLICKED: apply filter and save output
                        apply_filter_and_save(i);
                    }

                    if (i == selectedFilter) {
                        DrawRectangleRec(item, Fade(DARKGRAY, 0.4f)); // highlight overlay
                    }
                }
                EndScissorMode();

                // --- Templates ---
                DrawRectangleRec(templatesPanel, Fade(DARKGRAY, 0.02f));
                DrawText("Templates", templatesPanel.x, templatesPanel.y, 24, DARKBLUE); // bigger
                Rectangle tempView = { templatesPanel.x, templatesPanel.y + 40, templatesPanel.width, templatesPanel.height - 50 };
                Rectangle tempContent = { 0, 0, tempView.width - 20, templateCount * 40 }; // bigger items
                static Vector2 tempScroll = {0};
                GuiScrollPanel(tempView, NULL, tempContent, &tempScroll, NULL);
                BeginScissorMode(tempView.x, tempView.y, tempView.width, tempView.height);
                for (int i = 0; i < templateCount; i++) {
                    Rectangle item = { tempView.x, tempView.y + i*40 + (int)tempScroll.y, tempView.width, 40 };

                    if (GuiButton(item, templateOptions[i])) {
                        selectedTemplate = i;
                        selectedFilter = -1; // deselect filter
                    }

                    if (i == selectedTemplate) {
                        DrawRectangleRec(item, Fade(DARKGRAY, 0.4f)); // highlight overlay
                    }
                }
                EndScissorMode();

                break;
            }
        }

        // Draw GUI status / messages in right panel area
        DrawText("Status:", rightPanel.x + 8, rightPanel.y + 150, 14, DARKGRAY);
        // If there's an error message, draw it in red (simple heuristic: message contains "Failed" or "not")
        Color msgColor = BLACK;
        if (strstr(guiMessage, "Failed") || strstr(guiMessage, "failed") || strstr(guiMessage, "not") || strstr(guiMessage, "No image") || strstr(guiMessage, "empty")) msgColor = RED;
        DrawText(guiMessage, rightPanel.x + 8, rightPanel.y + 170, 12, msgColor);

        EndDrawing();
    }

    // Cleanup
    free_pixels();
    CloseWindow();
    return 0;
}
