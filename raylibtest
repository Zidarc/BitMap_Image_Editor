#include "raylib.h"
#include <stdio.h>
#include <string.h>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

typedef enum GameScreen { HOME, LOGIN, SIGNUP, MAIN } GameScreen;

int main() {
    // Window setup
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "BMT Image Project");

    SetTargetFPS(60);

    GameScreen currentScreen = HOME;

    // Input variables
    char username[32] = "";
    char password[32] = "";
    char confirmPassword[32] = "";
    char email[64] = "";
    char loggedInUser[32] = "Guest";

    // Active textbox tracker (0=none, LOGIN: 1=username,2=password, SIGNUP:1-username,2-password,3-confirm,4-email)
    int activeTextbox = 0;

    // Main screen variables
    Rectangle addButton = {50, 100, 50, 50};
    Rectangle websiteButton = {120, 100, 150, 50};
    Rectangle dropdown = {50, 180, 200, 30};
    int dropdownOption = 0;
    const char *editOptions[] = {"Resize", "Crop", "Rotate"};
    bool showDropdown = false;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch (currentScreen) {
        case HOME:
            DrawText("Welcome to BMT Image Project", 200, 50, 20, DARKBLUE);

            if (GuiButton((Rectangle){300, 200, 200, 50}, "Login")) {
                currentScreen = LOGIN;
                strcpy(username, "");
                strcpy(password, "");
                activeTextbox = 0;
            }

            if (GuiButton((Rectangle){300, 300, 200, 50}, "Sign Up")) {
                currentScreen = SIGNUP;
                strcpy(username, "");
                strcpy(password, "");
                strcpy(confirmPassword, "");
                strcpy(email, "");
                activeTextbox = 0;
            }

            if (GuiButton((Rectangle){300, 400, 200, 50}, "Continue as Guest")) {
                currentScreen = MAIN;
                strcpy(loggedInUser, "Guest");
            }
            break;

        case LOGIN:
            DrawText("Login", 350, 50, 30, DARKGREEN);

            DrawText("Username:", 200, 150, 20, BLACK);
            if (GuiTextBox((Rectangle){320, 145, 250, 30}, username, 32, activeTextbox == 1)) activeTextbox = 1;

            DrawText("Password:", 200, 200, 20, BLACK);
            if (GuiTextBox((Rectangle){320, 195, 250, 30}, password, 32, activeTextbox == 2)) activeTextbox = 2;

            if (GuiButton((Rectangle){320, 250, 100, 40}, "Login")) {
                strcpy(loggedInUser, username);
                currentScreen = MAIN;
                activeTextbox = 0;
            }

            if (GuiButton((Rectangle){430, 250, 150, 40}, "Forgot Password")) {
                // Simulate forgot password
            }

            if (GuiButton((Rectangle){320, 300, 260, 40}, "Back")) {
                currentScreen = HOME;
                activeTextbox = 0;
            }
            break;

        case SIGNUP:
            DrawText("Sign Up", 330, 50, 30, DARKPURPLE);

            DrawText("Username:", 200, 120, 20, BLACK);
            if (GuiTextBox((Rectangle){350, 115, 250, 30}, username, 32, activeTextbox == 1)) activeTextbox = 1;

            DrawText("Password:", 200, 170, 20, BLACK);
            if (GuiTextBox((Rectangle){350, 165, 250, 30}, password, 32, activeTextbox == 2)) activeTextbox = 2;

            DrawText("Confirm Pass:", 200, 220, 20, BLACK);
            if (GuiTextBox((Rectangle){350, 215, 250, 30}, confirmPassword, 32, activeTextbox == 3)) activeTextbox = 3;

            DrawText("Email:", 200, 270, 20, BLACK);
            if (GuiTextBox((Rectangle){350, 265, 250, 30}, email, 64, activeTextbox == 4)) activeTextbox = 4;

            if (GuiButton((Rectangle){350, 320, 100, 40}, "Sign Up")) {
                strcpy(loggedInUser, username);
                currentScreen = MAIN;
                activeTextbox = 0;
            }

            if (GuiButton((Rectangle){460, 320, 100, 40}, "Back")) {
                currentScreen = HOME;
                activeTextbox = 0;
            }
            break;

        case MAIN:
            DrawText("BMT Home", 350, 20, 30, DARKBLUE);
            DrawText(loggedInUser, 650, 20, 20, BLACK);

            // Add image button
            DrawRectangleRec(addButton, LIGHTGRAY);
            DrawText("+", addButton.x + 15, addButton.y + 10, 30, BLACK);
            if (CheckCollisionPointRec(GetMousePosition(), addButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                // Simulate image adding
            }

            // Website link button
            if (GuiButton(websiteButton, "Add from Website")) {
                // Simulate adding from website
            }

            // Dropdown menu
            DrawRectangleRec(dropdown, LIGHTGRAY);
            DrawText(editOptions[dropdownOption], dropdown.x + 5, dropdown.y + 5, 20, BLACK);
            if (CheckCollisionPointRec(GetMousePosition(), dropdown) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                showDropdown = !showDropdown;
            }

            if (showDropdown) {
                for (int i = 0; i < 3; i++) {
                    Rectangle optionRec = {dropdown.x, dropdown.y + 30 * (i + 1), dropdown.width, 30};
                    DrawRectangleRec(optionRec, GRAY);
                    DrawText(editOptions[i], optionRec.x + 5, optionRec.y + 5, 20, BLACK);
                    if (CheckCollisionPointRec(GetMousePosition(), optionRec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        dropdownOption = i;
                        showDropdown = false;
                    }
                }
            }

            DrawText("Generated link: www.bmt.com/yourimage", 50, 400, 20, DARKGREEN);
            break;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
