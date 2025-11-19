// bitmapgui.c
// Raylib + RayGUI GUI for BMT Image Project (modified - image code removed)
// Image/texture/history operations removed (Option B) — UI retained.

#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// --------------------- Utility globals ---------------------

static int screenWidth = 1100;
static int screenHeight = 700;

static Rectangle leftPanel = {10, 40, 220, 640};
static Rectangle rightPanel = {840, 40, 250, 640};
static Rectangle centerArea = {240, 40, 590, 640};

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
};
static const int editOptionsCount = sizeof(editOptions)/sizeof(editOptions[0]);
static int selectedEditOption = 0;
static bool showEditDropdown = false;

// simple numeric inputs for resize
static int resizeW = 320;
static int resizeH = 240;

// --------------------- Simple helpers ---------------------

static void SetError(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(errorMsg, sizeof(errorMsg), fmt, args);
    va_end(args);
    showErrorModal = true;
}

static void SetStatus(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(statusLine, sizeof(statusLine), fmt, args);
    va_end(args);
}

// --------------------- Image-related operations (REMOVED) ---------------------

// NOTE: All image/texture/history logic was intentionally removed in this build.
// Functions that previously loaded/saved/updated textures are no longer present.
// The UI still contains controls but any image operation will show an explanatory error.

// Replaced DoLoadImage with a stub that informs user image features are disabled.
static void DoLoadImage() {
    SetError("Image loading has been disabled in this build (Option B).");
}

// ApplyEditOption now only reports that image operations were removed.
static void ApplyEditOption(int option) {
    SetError("Image edit operations have been removed in this build (Option B).");
}

// --------------------- UI Drawing ---------------------

int main(void) {
    InitWindow(screenWidth, screenHeight, "BMT Image Project - GUI (No Image Support)");
    SetTargetFPS(60);

    // UI state
    bool running = true;
    bool leftPanelOpen = true;

    while (!WindowShouldClose() && running) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Top menu bar
        DrawRectangle(0, 0, screenWidth, 36, LIGHTGRAY);
        DrawText("BMT Editor - Layout C (No Image Support)", 12, 8, 14, DARKBLUE);
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

        // History toggle (history UI retained but will say disabled)
        if (GuiButton((Rectangle){leftPanel.x + 8, leftPanel.y + 128, leftPanel.width - 16, 28}, historyVisible ? "Hide History" : "Show History")) {
            historyVisible = !historyVisible;
        }

        // Edit Options (Scrollable)
        DrawText("Edit options:", leftPanel.x + 8, leftPanel.y + 170, 12, BLACK);

        // Scroll panel area
        Rectangle view = {
            leftPanel.x + 8,
            leftPanel.y + 190,
            leftPanel.width - 16,
            150  // visible height
        };

        // Full content height (all items)
        Rectangle content = {
            0,
            0,
            view.width - 20,
            editOptionsCount * 28
        };

        static Vector2 scroll = {0};
        GuiScrollPanel(view, NULL, content, &scroll, NULL);

        BeginScissorMode(view.x, view.y, view.width, view.height);

        for (int i = 0; i < editOptionsCount; i++) {
            Rectangle item = {
                view.x,
                view.y + i * 28 + (int)scroll.y,
                view.width,
                28
            };

            bool isSelected = (i == selectedEditOption);

            if (isSelected) {
                DrawRectangleRec(item, BLACK);
                DrawText(editOptions[i], item.x + 4, item.y + 6, 12, WHITE);

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
                    CheckCollisionPointRec(GetMousePosition(), item)) {
                    selectedEditOption = i;
                }
            } else {
                if (GuiButton(item, editOptions[i])) {
                    selectedEditOption = i;
                }
            }
        }

        EndScissorMode();

        // Resize inputs
        DrawText("Resize (W x H):", leftPanel.x + 8, leftPanel.y + 360, 12, BLACK);
        GuiSpinner((Rectangle){leftPanel.x + 8, leftPanel.y + 380, (leftPanel.width - 24)/2, 28}, "W", &resizeW, 1, 16384,false);
        GuiSpinner((Rectangle){leftPanel.x + 16 + (leftPanel.width - 24)/2, leftPanel.y + 380, (leftPanel.width - 24)/2, 28}, "H", &resizeH, 1, 16384,false);

        if (GuiButton((Rectangle){leftPanel.x + 8, leftPanel.y + 420, leftPanel.width - 16, 28}, "Apply Selected Edit")) {
            ApplyEditOption(selectedEditOption);
        }

        // Generate share link
        if (GuiButton((Rectangle){leftPanel.x + 8, leftPanel.y + 460, leftPanel.width - 16, 28}, "Generate Share Link")) {
            SetStatus("Generated link: www.bmt.com/yourimage/%08X", (unsigned int)GetTime());
        }

        // Right panel (history / info)
        DrawRectangleRec(rightPanel, Fade(DARKGRAY, 0.06f));
        GuiLabel((Rectangle){rightPanel.x + 8, rightPanel.y + 8, rightPanel.width - 16, 20}, "History");
        if (historyVisible) {
            DrawText("History disabled in this build.", rightPanel.x + 12, rightPanel.y + 40, 12, GRAY);
        } else {
            DrawText("History hidden", rightPanel.x + 12, rightPanel.y + 40, 12, GRAY);
        }

        // Center area: Image display (removed)
        DrawRectangleRec(centerArea, Fade(LIGHTGRAY, 0.02f));
        DrawText("Image functionality removed", centerArea.x + 8, centerArea.y + 8, 12, DARKBLUE);

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

    CloseWindow();
    return 0;
}
