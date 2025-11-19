#include "raylib.h"
#include <stdio.h>
#include <string.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// Login functions from login.c
int signup(const char *username, const char *password);
int login(const char *username, const char *password);

typedef enum GameScreen { LOGIN_SCREEN, SIGNUP_SCREEN, MAIN_SCREEN } GameScreen;

// ------------------------------------------------------
// MAIN GUI PANEL SETTINGS
// ------------------------------------------------------
static int screenWidth = 1300;
static int screenHeight = 700;

static Rectangle leftPanel   = {10, 40, 240, 640};     // HISTORY
static Rectangle centerPanel = {260, 40, 760, 640};    // FILTERS + TEMPLATES
static Rectangle rightPanel  = {1040, 40, 250, 640};   // LOAD IMAGE

// Filter and Template lists
static const char *filterOptions[] = {
    "Resize", "Rotate 90°", "Rotate 180°", "Flip Horizontal",
    "Flip Vertical", "Grayscale", "Invert"
};
static int filterCount = sizeof(filterOptions)/sizeof(filterOptions[0]);
static int selectedFilter = -1;

static const char *templateOptions[] = {
    "Facebook","Instagram","YouTube","Twitter",
    "TikTok","Snapchat","LinkedIn"
};
static int templateCount = sizeof(templateOptions)/sizeof(templateOptions[0]);
static int selectedTemplate = -1;

// Load image path
static char pathInput[512] = "";

int main() {
    InitWindow(screenWidth, screenHeight, "BMT Image Project");
    SetTargetFPS(60);

    GameScreen currentScreen = LOGIN_SCREEN;

    // Login/signup variables
    char username[32] = "";
    char password[32] = "";
    char confirmPassword[32] = "";
    bool showPassword = false;
    char loggedInUser[32] = "";

    // Textbox focus tracking
    int textBoxActive = 0; // 0=username, 1=password, 2=confirmPassword (signup)

    while (!WindowShouldClose()) {
        // ---------------- Input Handling ----------------
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
                        strcpy(loggedInUser, username);
                        currentScreen = MAIN_SCREEN;
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
                        DrawText("Passwords do not match!", 350, 370, 16, RED);
                    } else if (signup(username, password)) {
                        strcpy(username, "");
                        strcpy(password, "");
                        strcpy(confirmPassword, "");
                        textBoxActive = 0;
                        currentScreen = LOGIN_SCREEN;
                    } else {
                        DrawText("Sign up failed! Username may exist.", 350, 370, 16, RED);
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
                GuiTextBox((Rectangle){ rightPanel.x + 8, rightPanel.y + 70, rightPanel.width - 16, 30 }, pathInput, sizeof(pathInput), true);
                if (GuiButton((Rectangle){ rightPanel.x + 8, rightPanel.y + 110, rightPanel.width - 16, 30 }, "Load")) {
                    // Image loading disabled
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
                    bool isSelected = (i == selectedFilter);
                    if (isSelected) DrawRectangleRec(item, DARKGRAY);
                    if (GuiButton(item, filterOptions[i])) {
                        selectedFilter = i;
                        selectedTemplate = -1; // deselect template
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
                    if (i == selectedTemplate) DrawRectangleRec(item, DARKGRAY);
                    if (GuiButton(item, templateOptions[i])) {
                        selectedTemplate = i;
                        selectedFilter = -1; // deselect filter
                    }
                }
                EndScissorMode();
                break;
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
