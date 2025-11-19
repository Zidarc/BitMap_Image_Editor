// bitmapgui.c - 3-panel layout (Option A: 2 vertical bars in center)
// Image functionality removed
// No error popups
// Only one selection allowed at a time
// Bigger Filters and Templates visually

#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// ------------------------------------------------------
// WINDOW + PANELS
// ------------------------------------------------------

static int screenWidth = 1300;
static int screenHeight = 700;

static Rectangle leftPanel   = {10, 40, 240, 640};     // HISTORY
static Rectangle centerPanel = {260, 40, 760, 640};    // FILTERS + TEMPLATES ONLY
static Rectangle rightPanel  = {1040, 40, 250, 640};   // LOAD IMAGE

// ------------------------------------------------------
// STATE
// ------------------------------------------------------

// Path input box
static char pathInput[512] = "";

// ---------------- FILTER LIST ----------------
static const char *filterOptions[] = {
    "Resize",
    "Rotate 90°",
    "Rotate 180°",
    "Flip Horizontal",
    "Flip Vertical",
    "Grayscale",
    "Invert",
};
static int filterCount = sizeof(filterOptions)/sizeof(filterOptions[0]);
static int selectedFilter = -1;

// ---------------- TEMPLATE LIST ----------------
static const char *templateOptions[] = {
    "Facebook",
    "Instagram",
    "YouTube",
    "Twitter",
    "TikTok",
    "Snapchat",
    "LinkedIn",
};
static int templateCount = sizeof(templateOptions)/sizeof(templateOptions[0]);
static int selectedTemplate = -1;

// ------------------------------------------------------
// HELPERS
// ------------------------------------------------------

static void DoLoadImage() {
    // Image loading disabled
}

static void ApplyFilter() {
    // Filter operations disabled
}

// ------------------------------------------------------
// MAIN
// ------------------------------------------------------

int main(void) {
    InitWindow(screenWidth, screenHeight, "Custom GUI - Option A Layout");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // TOP BAR
        DrawRectangle(0, 0, screenWidth, 36, LIGHTGRAY);
        DrawText("Custom GUI (Image system disabled)", 12, 8, 14, DARKBLUE);

        // ------------------------------------------------------
        // LEFT PANEL — HISTORY
        // ------------------------------------------------------
        DrawRectangleRec(leftPanel, Fade(DARKGRAY, 0.08f));
        DrawText("History", leftPanel.x + 8, leftPanel.y + 8, 20, DARKGRAY);
        DrawText("History disabled", leftPanel.x + 12, leftPanel.y + 50, 14, GRAY);
        DrawText("in Option B mode.", leftPanel.x + 12, leftPanel.y + 70, 14, GRAY);

        // ------------------------------------------------------
        // RIGHT PANEL — LOAD IMAGE
        // ------------------------------------------------------
        DrawRectangleRec(rightPanel, Fade(DARKGRAY, 0.08f));
        DrawText("Load Image", rightPanel.x + 8, rightPanel.y + 8, 20, DARKGRAY);

        DrawText("Path:", rightPanel.x + 8, rightPanel.y + 50, 14, DARKGRAY);
        GuiTextBox((Rectangle){ rightPanel.x + 8, rightPanel.y + 70,
                               rightPanel.width - 16, 30 },
                   pathInput, sizeof(pathInput), true);

        if (GuiButton((Rectangle){ rightPanel.x + 8, rightPanel.y + 110,
                                   rightPanel.width - 16, 30 }, "Load"))
        {
            DoLoadImage();
        }

        // ------------------------------------------------------
        // CENTER PANEL — FILTERS + TEMPLATES
        // ------------------------------------------------------
        DrawRectangleRec(centerPanel, Fade(DARKGRAY, 0.02f));

        Rectangle filtersPanel = { centerPanel.x + 10, centerPanel.y + 10, (centerPanel.width - 30)/2, centerPanel.height - 20 };
        Rectangle templatesPanel = { centerPanel.x + 20 + (centerPanel.width - 30)/2, centerPanel.y + 10, (centerPanel.width - 30)/2, centerPanel.height - 20 };

        // ----------------- FILTERS -----------------
        DrawRectangleRec(filtersPanel, Fade(DARKGRAY, 0.02f));
        DrawText("Filters", filtersPanel.x, filtersPanel.y, 28, DARKBLUE);

        Rectangle filterView = { filtersPanel.x, filtersPanel.y + 40, filtersPanel.width, filtersPanel.height - 50 };
        int filterItemHeight = 50;  // taller items
        Rectangle filterContent = { 0, 0, filterView.width - 20, filterCount * filterItemHeight };
        static Vector2 filterScroll = {0};

        GuiScrollPanel(filterView, NULL, filterContent, &filterScroll, NULL);
        BeginScissorMode(filterView.x, filterView.y, filterView.width, filterView.height);

        for (int i = 0; i < filterCount; i++) {
            Rectangle item = { filterView.x, filterView.y + i * filterItemHeight + (int)filterScroll.y, filterView.width, filterItemHeight };
            bool isSelected = (i == selectedFilter);

            // Draw manually for bigger visual
            DrawRectangleRec(item, isSelected ? DARKGRAY : Fade(LIGHTGRAY, 0.3f));
            DrawText(filterOptions[i], item.x + 8, item.y + 12, 20, isSelected ? WHITE : BLACK);

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
                CheckCollisionPointRec(GetMousePosition(), item)) {
                selectedTemplate = -1; // deselect template
                selectedFilter = i;
                ApplyFilter();
            }
        }

        EndScissorMode();

        // ----------------- TEMPLATES -----------------
        DrawRectangleRec(templatesPanel, Fade(DARKGRAY, 0.02f));
        DrawText("Templates", templatesPanel.x, templatesPanel.y, 28, DARKBLUE);

        Rectangle tempView = { templatesPanel.x, templatesPanel.y + 40, templatesPanel.width, templatesPanel.height - 50 };
        int tempItemHeight = 50;
        Rectangle tempContent = { 0, 0, tempView.width - 20, templateCount * tempItemHeight };
        static Vector2 tempScroll = {0};

        GuiScrollPanel(tempView, NULL, tempContent, &tempScroll, NULL);
        BeginScissorMode(tempView.x, tempView.y, tempView.width, tempView.height);

        for (int i = 0; i < templateCount; i++) {
            Rectangle item = { tempView.x, tempView.y + i * tempItemHeight + (int)tempScroll.y, tempView.width, tempItemHeight };
            bool isSelected = (i == selectedTemplate);

            DrawRectangleRec(item, isSelected ? DARKGRAY : Fade(LIGHTGRAY, 0.3f));
            DrawText(templateOptions[i], item.x + 8, item.y + 12, 20, isSelected ? WHITE : BLACK);

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
                CheckCollisionPointRec(GetMousePosition(), item)) {
                selectedFilter = -1; // deselect filter
                selectedTemplate = i;
            }
        }

        EndScissorMode();

        // ------------------------------------------------------
        // FOOTER
        // ------------------------------------------------------
        DrawRectangle(0, screenHeight - 28, screenWidth, 28, LIGHTGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
