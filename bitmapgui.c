// main.c
// Raylib + RayGUI GUI for BMT Image Project (Layout C - panel layout)
// Demo-mode by default. To integrate real BMT engine, define USE_PARTNER_API
// and provide the partner's bmp.h / implementations.
//
// Example compile (demo mode):
//   gcc main.c -o bmt -lraylib -lraygui -lm
//
// If you and partner want to integrate real loader/saver, add -DUSE_PARTNER_API
// and make sure the partner provides the functions described in the comments.

#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// --------------------- Partner API (optional) ---------------------
// If you want to use your own BMP engine, define USE_PARTNER_API and
// implement these functions in some other file (partner's code).
// The GUI will call these functions when available.
//
// Expected semantics (example):
//   // load image from path, returning allocated pixels in RGBA (8-bit per channel).
//   // returns NULL on error (and sets errorMsg if you want).
//   unsigned char *partner_load_image(const char *path, int *outW, int *outH);
//   // save image pixels (RGBA) to file, return true on success.
//   bool partner_save_image(const char *path, unsigned char *pixels, int w, int h);
//   // Optionally operations:
//   void partner_invert(unsigned char *pixels, int w, int h); // in-place
//
// For demo compile, these are not required.
#ifdef USE_PARTNER_API
// #include "bmp.h"   // your partner's header
extern unsigned char *partner_load_image(const char *path, int *outW, int *outH);
extern bool partner_save_image(const char *path, unsigned char *pixels, int w, int h);
extern void partner_invert(unsigned char *pixels, int w, int h);
#endif

// --------------------- Utility types and globals ---------------------

typedef struct {
    int w, h;
    unsigned char *pixels; // RGBA, length = w*h*4
} ImageData;

typedef struct {
    int capacity;
    int size;
    ImageData *items;
    int currentIndex; // points to current image in history (for undo/redo)
} History;

// GUI state
static int screenWidth = 1100;
static int screenHeight = 700;

static Rectangle leftPanel = {10, 40, 220, 640};
static Rectangle rightPanel = {840, 40, 250, 640};
static Rectangle centerArea = {240, 40, 590, 640};

static ImageData currentImage = {0,0,NULL};
static Texture2D currentTexture = {0};

static History history = {0};

static bool showHistoryPanel = true;
static bool showErrorModal = false;
static char errorMsg[256] = "";
static char statusLine[256] = "Ready";

static char pathInput[512] = ""; // file path input
static bool historyVisible = true;

// dropdown options
static const char *editOptions[] = {
    "Resize",
    "Crop (not yet)",
    "Rotate 90°",
    "Rotate 180°",
    "Flip Horizontal",
    "Flip Vertical",
    "Grayscale",
    "Invert (Negative)",
    "Blur (not yet)",
    "Sharpen (not yet)",
    "Brightness/Contrast (not yet)",
    "Save Image"
};
static const int editOptionsCount = sizeof(editOptions)/sizeof(editOptions[0]);
static int selectedEditOption = 0;
static bool showEditDropdown = false;

// simple numeric inputs for resize
static int resizeW = 320;
static int resizeH = 240;

// --------------------- Memory helpers ---------------------

static void SetError(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(errorMsg, sizeof(errorMsg), fmt, args);
    va_end(args);
    showErrorModal = true;
}

static unsigned char *AllocPixels(int w, int h) {
    return (unsigned char*)malloc((size_t)w * h * 4);
}

static void FreeImageData(ImageData *img) {
    if (!img) return;
    if (img->pixels) free(img->pixels);
    img->pixels = NULL;
    img->w = img->h = 0;
}

// copy pixels (RGBA)
static unsigned char *CopyPixels(const unsigned char *src, int w, int h) {
    size_t bytes = (size_t)w * h * 4;
    unsigned char *dst = (unsigned char*)malloc(bytes);
    if (!dst) return NULL;
    memcpy(dst, src, bytes);
    return dst;
}

// --------------------- History management ---------------------

static void HistoryInit(History *h) {
    h->capacity = 8;
    h->size = 0;
    h->items = (ImageData*)malloc(sizeof(ImageData) * h->capacity);
    for (int i=0;i<h->capacity;i++){ h->items[i].pixels = NULL; h->items[i].w=0; h->items[i].h=0; }
    h->currentIndex = -1;
}

static void HistoryFree(History *h) {
    if (!h->items) return;
    for (int i=0;i<h->size;i++) FreeImageData(&h->items[i]);
    free(h->items);
    h->items = NULL;
    h->size = 0;
    h->capacity = 0;
    h->currentIndex = -1;
}

static void HistoryPush(History *h, const ImageData *img) {
    // If we're not at the end of history, drop forward states (invalidate redo)
    if (h->currentIndex < h->size - 1) {
        // free forward states
        for (int i = h->currentIndex + 1; i < h->size; i++) FreeImageData(&h->items[i]);
        h->size = h->currentIndex + 1;
    }
    // ensure capacity
    if (h->size >= h->capacity) {
        int newCap = h->capacity * 2;
        ImageData *newItems = (ImageData*)realloc(h->items, sizeof(ImageData) * newCap);
        for (int i = h->capacity; i < newCap; i++) { newItems[i].pixels = NULL; newItems[i].w=0; newItems[i].h=0; }
        h->items = newItems;
        h->capacity = newCap;
    }
    // push copy
    h->items[h->size].w = img->w;
    h->items[h->size].h = img->h;
    h->items[h->size].pixels = CopyPixels(img->pixels, img->w, img->h);
    h->size++;
    h->currentIndex = h->size - 1;
}

static bool HistoryCanUndo(History *h) { return h->currentIndex > 0; }
static bool HistoryCanRedo(History *h) { return h->currentIndex < h->size - 1; }

static bool HistoryUndo(History *h, ImageData *out) {
    if (!HistoryCanUndo(h)) return false;
    h->currentIndex--;
    out->w = h->items[h->currentIndex].w;
    out->h = h->items[h->currentIndex].h;
    // replace out pixels with a copy
    if (out->pixels) { free(out->pixels); out->pixels = NULL; }
    out->pixels = CopyPixels(h->items[h->currentIndex].pixels, out->w, out->h);
    return true;
}
static bool HistoryRedo(History *h, ImageData *out) {
    if (!HistoryCanRedo(h)) return false;
    h->currentIndex++;
    out->w = h->items[h->currentIndex].w;
    out->h = h->items[h->currentIndex].h;
    if (out->pixels) { free(out->pixels); out->pixels = NULL; }
    out->pixels = CopyPixels(h->items[h->currentIndex].pixels, out->w, out->h);
    return true;
}

// --------------------- Demo image generator ---------------------

static void GenerateDemoCheckerboard(ImageData *out, int w, int h) {
    if (out->pixels) free(out->pixels);
    out->w = w; out->h = h;
    out->pixels = AllocPixels(w, h);
    if (!out->pixels) return;
    for (int y=0;y<h;y++){
        for (int x=0;x<w;x++){
            int idx = (y * w + x) * 4;
            int check = ((x/16) + (y/16)) & 1;
            if (check==0) {
                out->pixels[idx+0] = 200; // r
                out->pixels[idx+1] = 200; // g
                out->pixels[idx+2] = 200; // b
                out->pixels[idx+3] = 255;
            } else {
                out->pixels[idx+0] = 100;
                out->pixels[idx+1] = 150;
                out->pixels[idx+2] = 220;
                out->pixels[idx+3] = 255;
            }
        }
    }
}

// --------------------- Texture helpers ---------------------

static void UnloadCurrentTexture() {
    if (currentTexture.id != 0) {
        UnloadTexture(currentTexture);
        currentTexture.id = 0;
    }
}

static void UpdateTextureFromImageData(ImageData *img) {
    if (!img || !img->pixels) return;
    // If sizes mismatch, recreate texture
    if (currentTexture.id == 0 || currentTexture.width != img->w || currentTexture.height != img->h) {
        UnloadCurrentTexture();
        // Build a temporary Image struct to load texture
        Image temp = {
            .data = img->pixels,
            .width = img->w,
            .height = img->h,
            .mipmaps = 1,
            .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
        };
        // Note: LoadTextureFromImage copies the pixels internally in raylib
        currentTexture = LoadTextureFromImage(temp);
    } else {
        UpdateTexture(currentTexture, img->pixels);
    }
}

// --------------------- Basic pixel ops (in-place) ---------------------

static void OperationInvert(ImageData *img) {
#ifdef USE_PARTNER_API
    // If partner provides an in-place invert, call it
    if (partner_invert) {
        partner_invert(img->pixels, img->w, img->h);
        return;
    }
#endif
    // Demo invert (RGBA)
    size_t len = (size_t)img->w * img->h * 4;
    for (size_t i=0;i<len;i+=4){
        img->pixels[i+0] = 255 - img->pixels[i+0];
        img->pixels[i+1] = 255 - img->pixels[i+1];
        img->pixels[i+2] = 255 - img->pixels[i+2];
        // keep alpha
    }
}

static void OperationGrayscale(ImageData *img) {
    size_t len = (size_t)img->w * img->h * 4;
    for (size_t i=0;i<len;i+=4){
        unsigned char r = img->pixels[i+0];
        unsigned char g = img->pixels[i+1];
        unsigned char b = img->pixels[i+2];
        unsigned char y = (unsigned char)((0.299*r + 0.587*g + 0.114*b));
        img->pixels[i+0] = img->pixels[i+1] = img->pixels[i+2] = y;
    }
}

static void OperationFlipHorizontal(ImageData *img) {
    int w = img->w, h = img->h;
    unsigned char *temp = (unsigned char*)malloc((size_t)w * 4);
    for (int y=0;y<h;y++){
        unsigned char *row = img->pixels + (size_t)y * w * 4;
        for (int x=0;x<w/2;x++){
            unsigned char *a = row + x*4;
            unsigned char *b = row + (w-1-x)*4;
            for (int k=0;k<4;k++){
                unsigned char t = a[k];
                a[k] = b[k];
                b[k] = t;
            }
        }
    }
    free(temp);
}

static void OperationFlipVertical(ImageData *img) {
    int w = img->w, h = img->h;
    size_t rowBytes = (size_t)w * 4;
    unsigned char *tempRow = (unsigned char*)malloc(rowBytes);
    for (int y=0;y<h/2;y++){
        unsigned char *rowA = img->pixels + (size_t)y * rowBytes;
        unsigned char *rowB = img->pixels + (size_t)(h-1-y) * rowBytes;
        memcpy(tempRow, rowA, rowBytes);
        memcpy(rowA, rowB, rowBytes);
        memcpy(rowB, tempRow, rowBytes);
    }
    free(tempRow);
}

// rotate 90 degrees clockwise (create new buffer)
static void OperationRotate90(ImageData *img) {
    int w = img->w, h = img->h;
    unsigned char *newpix = AllocPixels(h, w); // width becomes height
    for (int y=0;y<h;y++){
        for (int x=0;x<w;x++){
            int srcIdx = (y*w + x)*4;
            // dest coords (x',y') = (h-1 - y, x)
            int dstX = h - 1 - y;
            int dstY = x;
            int dstIdx = (dstY * h + dstX) * 4; // note new width = h
            newpix[dstIdx+0] = img->pixels[srcIdx+0];
            newpix[dstIdx+1] = img->pixels[srcIdx+1];
            newpix[dstIdx+2] = img->pixels[srcIdx+2];
            newpix[dstIdx+3] = img->pixels[srcIdx+3];
        }
    }
    free(img->pixels);
    img->pixels = newpix;
    int oldW = img->w;
    img->w = h;
    img->h = oldW;
}

// simple nearest-neighbor resize (in-place reallocate)
static void OperationResize(ImageData *img, int newW, int newH) {
    unsigned char *newpix = AllocPixels(newW, newH);
    for (int y=0;y<newH;y++){
        for (int x=0;x<newW;x++){
            int srcX = (int)((float)x * img->w / newW);
            int srcY = (int)((float)y * img->h / newH);
            if (srcX >= img->w) srcX = img->w - 1;
            if (srcY >= img->h) srcY = img->h - 1;
            int srcIdx = (srcY * img->w + srcX) * 4;
            int dstIdx = (y * newW + x) * 4;
            newpix[dstIdx+0] = img->pixels[srcIdx+0];
            newpix[dstIdx+1] = img->pixels[srcIdx+1];
            newpix[dstIdx+2] = img->pixels[srcIdx+2];
            newpix[dstIdx+3] = img->pixels[srcIdx+3];
        }
    }
    free(img->pixels);
    img->pixels = newpix;
    img->w = newW;
    img->h = newH;
}

// --------------------- Loading / Saving stubs ---------------------

// Attempt to load an image from path. If partner API is provided and used, calls it.
// Otherwise, creates a demo checkerboard.
static bool LoadImageFromPath(const char *path, ImageData *out) {
#ifdef USE_PARTNER_API
    int w=0,h=0;
    unsigned char *p = partner_load_image(path, &w, &h);
    if (!p) {
        return false;
    }
    if (out->pixels) free(out->pixels);
    out->w = w; out->h = h; out->pixels = p; // partner must allocate
    return true;
#else
    // Demo: ignore path, generate demo image sized 512x384
    GenerateDemoCheckerboard(out, 512, 384);
    return true;
#endif
}

static bool SaveImageToPath(const char *path, ImageData *img) {
#ifdef USE_PARTNER_API
    return partner_save_image(path, img->pixels, img->w, img->h);
#else
    // Demo: write a simple raw RGBA file for debugging
    FILE *f = fopen(path, "wb");
    if (!f) return false;
    fwrite(&img->w, sizeof(int), 1, f);
    fwrite(&img->h, sizeof(int), 1, f);
    fwrite(img->pixels, 1, (size_t)img->w * img->h * 4, f);
    fclose(f);
    return true;
#endif
}

// --------------------- High-level GUI actions ---------------------

static void SetStatus(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(statusLine, sizeof(statusLine), fmt, args);
    va_end(args);
}

static void DoLoadImage() {
    if (strlen(pathInput) == 0) {
        SetError("Please enter a file path or URL in the input box.");
        return;
    }
    ImageData newImg = {0,0,NULL};
    if (!LoadImageFromPath(pathInput, &newImg)) {
        SetError("Failed to load image from:\n%s", pathInput);
        return;
    }
    // unload current texture and image
    if (currentImage.pixels) free(currentImage.pixels);
    currentImage = newImg;
    UnloadCurrentTexture();
    UpdateTextureFromImageData(&currentImage);
    // push to history
    HistoryPush(&history, &currentImage);
    SetStatus("Loaded image: %s (%dx%d)", pathInput, currentImage.w, currentImage.h);
}

static void DoSaveImage() {
    if (!currentImage.pixels) { SetError("No image to save."); return; }
    if (strlen(pathInput) == 0) { SetError("Enter a path to save the image in the input box."); return; }
    if (!SaveImageToPath(pathInput, &currentImage)) {
        SetError("Failed to save image to:\n%s", pathInput);
        return;
    }
    SetStatus("Saved image to %s", pathInput);
}

static void ApplyEditOption(int option) {
    if (!currentImage.pixels) { SetError("Load an image first."); return; }
    // Save state before operation for undo
    HistoryPush(&history, &currentImage);

    switch(option) {
        case 0: // Resize
            OperationResize(&currentImage, resizeW, resizeH);
            SetStatus("Resized to %dx%d", currentImage.w, currentImage.h);
            break;
        case 1: // Crop - not implemented
            SetError("Crop is not implemented in GUI demo. Partner should implement.");
            break;
        case 2: // Rotate 90
            OperationRotate90(&currentImage);
            SetStatus("Rotated 90° (now %dx%d)", currentImage.w, currentImage.h);
            break;
        case 3: // Rotate 180
            OperationRotate90(&currentImage);
            OperationRotate90(&currentImage);
            SetStatus("Rotated 180°");
            break;
        case 4: // Flip H
            OperationFlipHorizontal(&currentImage);
            SetStatus("Flipped horizontally");
            break;
        case 5: // Flip V
            OperationFlipVertical(&currentImage);
            SetStatus("Flipped vertically");
            break;
        case 6: // Grayscale
            OperationGrayscale(&currentImage);
            SetStatus("Converted to grayscale");
            break;
        case 7: // Invert
            OperationInvert(&currentImage);
            SetStatus("Inverted colors");
            break;
        case 8: // Blur
        case 9: // Sharpen
        case 10: // Brightness/Contrast
            SetError("%s is not implemented yet. Partner can hook real implementation.", editOptions[option]);
            // pop last history entry because no change actually happened
            if (history.size > 0) { FreeImageData(&history.items[history.size-1]); history.size--; history.currentIndex = history.size-1; }
            return;
        case 11: // Save Image
            DoSaveImage();
            // pop history entry added earlier since save doesn't change pixels
            if (history.size > 0) { FreeImageData(&history.items[history.size-1]); history.size--; history.currentIndex = history.size-1; }
            return;
        default:
            SetError("Unknown operation.");
            return;
    }
    // After changing pixels, update texture
    UnloadCurrentTexture();
    UpdateTextureFromImageData(&currentImage);
}

static void DoUndo() {
    if (!HistoryCanUndo(&history)) {
        SetError("Nothing to undo.");
        return;
    }
    ImageData tmp = {0,0,NULL};
    if (HistoryUndo(&history, &tmp)) {
        if (currentImage.pixels) free(currentImage.pixels);
        currentImage = tmp;
        UnloadCurrentTexture();
        UpdateTextureFromImageData(&currentImage);
        SetStatus("Undo -> %dx%d", currentImage.w, currentImage.h);
    }
}

static void DoRedo() {
    if (!HistoryCanRedo(&history)) {
        SetError("Nothing to redo.");
        return;
    }
    ImageData tmp = {0,0,NULL};
    if (HistoryRedo(&history, &tmp)) {
        if (currentImage.pixels) free(currentImage.pixels);
        currentImage = tmp;
        UnloadCurrentTexture();
        UpdateTextureFromImageData(&currentImage);
        SetStatus("Redo -> %dx%d", currentImage.w, currentImage.h);
    }
}

// --------------------- UI Drawing ---------------------

int main(void) {
    InitWindow(screenWidth, screenHeight, "BMT Image Project - GUI (Panel Layout C)");
    SetTargetFPS(60);

    // init history
    HistoryInit(&history);

    // demo initial image
    GenerateDemoCheckerboard(&currentImage, 512, 384);
    UpdateTextureFromImageData(&currentImage);
    HistoryPush(&history, &currentImage);

    // UI state
    bool running = true;
    bool leftPanelOpen = true;

    while (!WindowShouldClose() && running) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Top menu bar
        DrawRectangle(0, 0, screenWidth, 36, LIGHTGRAY);
        DrawText("BMT Editor - Layout C", 12, 8, 14, DARKBLUE);
        DrawText(statusLine, 260, 8, 12, DARKGRAY);

        // Left panel (controls)
        DrawRectangleRec(leftPanel, Fade(DARKGRAY, 0.08f));
        GuiLabel((Rectangle){leftPanel.x + 8, leftPanel.y + 8, leftPanel.width - 16, 20}, "Controls");

        // File path input
        DrawText("Path or URL:", leftPanel.x + 8, leftPanel.y + 36, 12, BLACK);
        GuiTextBox((Rectangle){leftPanel.x + 8, leftPanel.y + 56, leftPanel.width - 16, 28}, pathInput, sizeof(pathInput), true);
        if (GuiButton((Rectangle){leftPanel.x + 8, leftPanel.y + 92, leftPanel.width - 16, 28}, "Load Image")) {
            DoLoadImage();
        }
        if (GuiButton((Rectangle){leftPanel.x + 8, leftPanel.y + 128, leftPanel.width - 16, 28}, "Save Image")) {
            DoSaveImage();
        }

        // Undo/Redo
        if (GuiButton((Rectangle){leftPanel.x + 8, leftPanel.y + 170, (leftPanel.width - 24)/2, 28}, "Undo")) {
            DoUndo();
        }
        if (GuiButton((Rectangle){leftPanel.x + 16 + (leftPanel.width - 24)/2, leftPanel.y + 170, (leftPanel.width - 24)/2, 28}, "Redo")) {
            DoRedo();
        }

        // History toggle
        if (GuiButton((Rectangle){leftPanel.x + 8, leftPanel.y + 208, leftPanel.width - 16, 28}, historyVisible ? "Hide History" : "Show History")) {
            historyVisible = !historyVisible;
        }

        // Edit Options (Scrollable)
		DrawText("Edit options:", leftPanel.x + 8, leftPanel.y + 250, 12, BLACK);

		// Scroll panel area
		Rectangle view = {
    		leftPanel.x + 8,
    		leftPanel.y + 270,
    		leftPanel.width - 16,
    		150  // visible height (fits ~5 items)
		};

		// Full content height (all items)
		Rectangle content = {
    		0,
    		0,
    		view.width - 20,         // width inside scroll
    		editOptionsCount * 28    // each row is 28 pixels tall
		};

		static Vector2 scroll = {0};

		// Draw scroll panel
		GuiScrollPanel(view, NULL, content, &scroll, NULL);

		// Clip to scroll view
		BeginScissorMode(view.x, view.y, view.width, view.height);

		// Draw items inside scrolled region
		for (int i = 0; i < editOptionsCount; i++) {
    		Rectangle item = {
        	view.x,
        	view.y + i * 28 + scroll.y,   // apply scroll offset
        	view.width,
        	28
    	};

    if (GuiButton(item, editOptions[i])) {
        selectedEditOption = i;
    }
}

EndScissorMode();


        // Resize inputs
        DrawText("Resize (W x H):", leftPanel.x + 8, leftPanel.y + 420, 12, BLACK);
        GuiSpinner((Rectangle){leftPanel.x + 8, leftPanel.y + 440, (leftPanel.width - 24)/2, 28}, "W", &resizeW, 1, 16384,false);
        GuiSpinner((Rectangle){leftPanel.x + 16 + (leftPanel.width - 24)/2, leftPanel.y + 440, (leftPanel.width - 24)/2, 28}, "H", &resizeH, 1, 16384,false);

        if (GuiButton((Rectangle){leftPanel.x + 8, leftPanel.y + 480, leftPanel.width - 16, 28}, "Apply Selected Edit")) {
            ApplyEditOption(selectedEditOption);
        }

        // Generate share link
        if (GuiButton((Rectangle){leftPanel.x + 8, leftPanel.y + 520, leftPanel.width - 16, 28}, "Generate Share Link")) {
            SetStatus("Generated link: www.bmt.com/yourimage/%08X", (unsigned int)GetTime());
        }

        // Right panel (history / info)
        DrawRectangleRec(rightPanel, Fade(DARKGRAY, 0.06f));
        GuiLabel((Rectangle){rightPanel.x + 8, rightPanel.y + 8, rightPanel.width - 16, 20}, "History");
        if (historyVisible) {
            // list history snapshots
            int startY = rightPanel.y + 36;
            for (int i = 0; i < history.size; i++) {
                int y = startY + i*28;
                char buf[64];
                snprintf(buf, sizeof(buf), "%d: %dx%d %s", i+1, history.items[i].w, history.items[i].h, (i==history.currentIndex?"<--":""));
                if (GuiButton((Rectangle){rightPanel.x + 8, y, rightPanel.width - 16, 24}, buf)) {
                    // jump to snapshot i
                    if (i >= 0 && i < history.size) {
                        if (currentImage.pixels) free(currentImage.pixels);
                        currentImage.w = history.items[i].w;
                        currentImage.h = history.items[i].h;
                        currentImage.pixels = CopyPixels(history.items[i].pixels, currentImage.w, currentImage.h);
                        UpdateTextureFromImageData(&currentImage);
                        history.currentIndex = i;
                        SetStatus("Jumped to history %d", i+1);
                    }
                }
            }
        } else {
            DrawText("History hidden", rightPanel.x + 12, rightPanel.y + 40, 12, GRAY);
        }

        // Center area: Image display
        DrawRectangleRec(centerArea, Fade(LIGHTGRAY, 0.02f));
        DrawText("Image Preview", centerArea.x + 8, centerArea.y + 8, 12, DARKBLUE);

        if (currentTexture.id != 0) {
            // Fit image inside center area with padding
            float pad = 12;
            float maxW = centerArea.width - pad*2;
            float maxH = centerArea.height - 36 - pad*2;
            float texW = (float)currentTexture.width;
            float texH = (float)currentTexture.height;
            float scale = fminf(maxW / texW, maxH / texH);
            if (scale > 1.0f) scale = 1.0f;
            float drawW = texW * scale;
            float drawH = texH * scale;
            float drawX = centerArea.x + (centerArea.width - drawW) / 2;
            float drawY = centerArea.y + 36 + (maxH - drawH) / 2;
            DrawTexturePro(currentTexture, (Rectangle){0,0,currentTexture.width, currentTexture.height}, (Rectangle){drawX, drawY, drawW, drawH}, (Vector2){0,0}, 0.0f, WHITE);
            DrawRectangleLinesEx((Rectangle){drawX-1, drawY-1, drawW+2, drawH+2}, 1, GRAY);
        } else {
            DrawText("No image loaded", centerArea.x + 16, centerArea.y + 60, 18, GRAY);
        }

        // Status / footer
        DrawRectangle(0, screenHeight - 28, screenWidth, 28, LIGHTGRAY);
        DrawText(statusLine, 8, screenHeight - 22, 12, DARKGRAY);

        // Error modal
        if (showErrorModal) {
            Rectangle r = { screenWidth/2 - 240, screenHeight/2 - 100, 480, 200 };
            DrawRectangleRec(r, Fade(BLACK, 0.7f));
            DrawRectangleRec((Rectangle){r.x+6, r.y+6, r.width-12, r.height-12}, RAYWHITE);
            DrawText("Error", r.x + 16, r.y + 12, 18, RED);
            DrawText(errorMsg, r.x + 16, r.y + 44, 12, DARKGRAY);
            if (GuiButton((Rectangle){r.x + r.width/2 - 60, r.y + r.height - 44, 120, 28}, "OK")) {
                showErrorModal = false;
            }
        }

        EndDrawing();
    }

    // cleanup
    UnloadCurrentTexture();
    if (currentImage.pixels) free(currentImage.pixels);
    HistoryFree(&history);
    CloseWindow();
    return 0;
}
