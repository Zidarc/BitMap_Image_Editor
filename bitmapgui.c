#include "raylib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "backend.h"   // Image loading/saving
#include "login.h"     // Login/signup
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// ---------------------- Game Screens ----------------------
typedef enum GameScreen { LOGIN_SCREEN, SIGNUP_SCREEN, MAIN_SCREEN } GameScreen;

// ---------------------- Window ----------------------
static int screenWidth = 1300;
static int screenHeight = 700;

// ---------------------- Panels ----------------------
static Rectangle leftPanel   = {10, 40, 240, 640};
static Rectangle centerPanel = {260, 40, 760, 640};
static Rectangle rightPanel  = {1040, 40, 250, 640};
static int rightTextBoxActive = 0;

// ---------------------- Filters/Templates ----------------------
static const char *filterOptions[] = {
    "Rotate 90°", "Rotate 180°", "Flip Horizontal",
    "Flip Vertical", "Grayscale", "Invert", "Sepia",
    "Blur", "Contrast", "Pixelate"
};
static int filterCount = sizeof(filterOptions)/sizeof(filterOptions[0]);
static int selectedFilter = -1;

static const char *templateOptions[] = {
    "Facebook","Instagram","YouTube","Twitter",
    "TikTok","Snapchat","LinkedIn"
};
static int templateCount = sizeof(templateOptions)/sizeof(templateOptions[0]);
static int selectedTemplate = -1;

// ---------------------- Image ----------------------
static char pathInput[512] = "";

// ---------------------- Brightness & Resize ----------------------
static char brightnessInput[8] = "0";
static int brightnessValue = 0;
static char newWidthInput[8] = "";
static char newHeightInput[8] = "";
static int newWidth = 0;
static int newHeight = 0;

// ---------------------- Error Pop-up ----------------------
static bool showErrorPopup = false;
static char errorMessage[256] = "";
static int errorTimer = 0;

// ---------------------- Main ----------------------
int main() {
    InitWindow(screenWidth, screenHeight, "BMT Image Project");
    SetTargetFPS(60);

    GameScreen currentScreen = LOGIN_SCREEN;

    // Login/signup
    char username[32] = "", password[32] = "", confirmPassword[32] = "", loggedInUser[32] = "";
    int textBoxActive = 0;

    // MAIN_SCREEN textbox focus
    int mainTextBoxActive = 0;
    int pathTextBoxActive = 0;
 // 0=brightness,1=width,2=height,3=path

    // Scrolls
    static Vector2 filterScroll = {0};
    static Vector2 tempScroll = {0};

    // Error pop-up timer (auto-hide after 3 seconds)
    if (errorTimer > 0) {
        errorTimer--;
        if (errorTimer == 0) showErrorPopup = false;
    }

    while (!WindowShouldClose()) {

        // ---------------- TAB NAVIGATION (Main screen only) ----------------
        if (IsKeyPressed(KEY_TAB)) {
            if(currentScreen==MAIN_SCREEN) {
                mainTextBoxActive = (mainTextBoxActive + 1) % 4;
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch(currentScreen) {

            // ---------------- LOGIN SCREEN ----------------
            case LOGIN_SCREEN:
                DrawText("Login", 350,50,30,DARKGREEN);

                DrawText("Username:",200,150,20,BLACK);
                if(GuiTextBox((Rectangle){320,145,250,30}, username,32,textBoxActive==0)) {
                    textBoxActive = 0;
                }

                DrawText("Password:",200,200,20,BLACK);
                if(GuiTextBox((Rectangle){320,195,250,30}, password,32,textBoxActive==1)) {
                    textBoxActive = 1;
                }

                if(GuiButton((Rectangle){320,250,100,40},"Login")) {
                    if(username[0] == '\0' || password[0] == '\0') {
                        strcpy(errorMessage, "Please enter both username and password");
                        showErrorPopup = true;
                        errorTimer = 180; // 3 seconds at 60 FPS
                    } else {
                        int result = login(username, password);
                        if(result == LOGIN_OK) {
                            strcpy(loggedInUser,username);
                            currentScreen=MAIN_SCREEN;
                        } else if(result == LOGIN_INVALID_CREDENTIALS) {
                            strcpy(errorMessage, "Invalid username or password");
                            showErrorPopup = true;
                            errorTimer = 180;
                        } else if(result == LOGIN_FILE_ERROR) {
                            strcpy(errorMessage, "Error accessing user database");
                            showErrorPopup = true;
                            errorTimer = 180;
                        }
                    }
                }
                if(GuiButton((Rectangle){430,250,150,40},"Sign Up")) {
                    strcpy(username,""); strcpy(password,""); strcpy(confirmPassword,""); textBoxActive=0;
                    currentScreen=SIGNUP_SCREEN;
                }
                break;

            // ---------------- SIGNUP SCREEN ----------------
            case SIGNUP_SCREEN:
                DrawText("Sign Up", 330,50,30,DARKPURPLE);

                DrawText("Username:",150,120,20,BLACK);
                if(GuiTextBox((Rectangle){350,115,250,30}, username,32,textBoxActive==0)) {
                    textBoxActive = 0;
                }

                DrawText("Password:",150,170,20,BLACK);
                if(GuiTextBox((Rectangle){350,165,250,30}, password,32,textBoxActive==1)) {
                    textBoxActive = 1;
                }

                DrawText("Confirm Password:",150,220,20,BLACK);
                if(GuiTextBox((Rectangle){350,215,250,30}, confirmPassword,32,textBoxActive==2)) {
                    textBoxActive = 2;
                }

                if(GuiButton((Rectangle){350,320,100,40},"Sign Up")) {
                    if(username[0] == '\0' || password[0] == '\0' || confirmPassword[0] == '\0') {
                        strcpy(errorMessage, "Please fill in all fields");
                        showErrorPopup = true;
                        errorTimer = 180;
                    } else if(strcmp(password, confirmPassword) != 0) {
                        strcpy(errorMessage, "Passwords do not match");
                        showErrorPopup = true;
                        errorTimer = 180;
                    } else {
                        int result = signup(username, password);
                        if(result == LOGIN_OK) {
                            currentScreen=LOGIN_SCREEN;
                            strcpy(username,""); strcpy(password,""); strcpy(confirmPassword,""); textBoxActive=0;
                        } else if(result == LOGIN_USERNAME_EXISTS) {
                            strcpy(errorMessage, "Username already exists");
                            showErrorPopup = true;
                            errorTimer = 180;
                        } else if(result == LOGIN_FILE_ERROR) {
                            strcpy(errorMessage, "Error accessing user database");
                            showErrorPopup = true;
                            errorTimer = 180;
                        }
                    }
                }
                if(GuiButton((Rectangle){460,320,100,40},"Back")) currentScreen=LOGIN_SCREEN;
                break;

            // ---------------- MAIN SCREEN ----------------
            case MAIN_SCREEN: {

                // Top bar
                DrawRectangle(0,0,screenWidth,36,LIGHTGRAY);
                DrawText("BMT Image Project",12,8,14,DARKBLUE);
                DrawText(loggedInUser,screenWidth-150,8,14,BLACK);

                // ---------------- Left Panel ----------------
                DrawRectangleRec(leftPanel, Fade(DARKGRAY,0.08f));
                DrawText("Adjustments", leftPanel.x+8, leftPanel.y+8,20,DARKBLUE);

                // Brightness
                DrawText("Brightness", leftPanel.x+8, leftPanel.y+40,18,DARKBLUE);
                GuiTextBox((Rectangle){leftPanel.x+8,leftPanel.y+70,leftPanel.width-16,30},
                           brightnessInput, 8, mainTextBoxActive==0);
                if(GuiButton((Rectangle){leftPanel.x+8,leftPanel.y+110,leftPanel.width-16,30},"Apply")){
                    brightnessValue = atoi(brightnessInput);
                    if(brightnessValue < -255 || brightnessValue > 255) {
                        strcpy(errorMessage, "Brightness must be between -255 and 255");
                        showErrorPopup = true;
                        errorTimer = 180;
                    } else {
                        backend_set_brightness(brightnessValue);
                        backend_apply_filter(11); // Brightness
                    }
                }

                // Resize
                DrawText("Resize", leftPanel.x+8, leftPanel.y+160,18,DARKBLUE);
                DrawText("Width:", leftPanel.x+8,leftPanel.y+190,14,BLACK);
                GuiTextBox((Rectangle){leftPanel.x+60,leftPanel.y+185,leftPanel.width-68,30},
                           newWidthInput,8,mainTextBoxActive==1);
                DrawText("Height:", leftPanel.x+8,leftPanel.y+230,14,BLACK);
                GuiTextBox((Rectangle){leftPanel.x+60,leftPanel.y+225,leftPanel.width-68,30},
                           newHeightInput,8,mainTextBoxActive==2);
                if(GuiButton((Rectangle){leftPanel.x+8,leftPanel.y+270,leftPanel.width-16,30},"Apply")){
                    newWidth = atoi(newWidthInput);
                    newHeight = atoi(newHeightInput);
                    if(newWidthInput[0] == '\0' || newHeightInput[0] == '\0') {
                        strcpy(errorMessage, "Please enter both width and height");
                        showErrorPopup = true;
                        errorTimer = 180;
                    } else if(newWidth <= 0 || newHeight <= 0) {
                        strcpy(errorMessage, "Width and height must be greater than 0");
                        showErrorPopup = true;
                        errorTimer = 180;
                    } else {
                        backend_set_resize(newWidth,newHeight);
                        backend_apply_filter(13); // Resize
                    }
                }

                // ---------------- Right Panel ----------------
                DrawRectangleRec(rightPanel, Fade(DARKGRAY,0.08f));
                DrawText("Load Image", rightPanel.x+8, rightPanel.y+8, 20, DARKGRAY);
                DrawText("Path:", rightPanel.x+8, rightPanel.y+50, 14, DARKGRAY);

                // Path textbox with active state
                pathTextBoxActive = GuiTextBox(
                    (Rectangle){rightPanel.x+8, rightPanel.y+70, rightPanel.width-16, 30},
                    pathInput, sizeof(pathInput), mainTextBoxActive==3
                );

                // Load button
                if(GuiButton((Rectangle){rightPanel.x+8, rightPanel.y+110, rightPanel.width-16, 30}, "Load")){
                    if(pathInput[0] == '\0') {
                        strcpy(errorMessage, "Please enter an image path");
                        showErrorPopup = true;
                        errorTimer = 180;
                    } else if(!backend_load_image(pathInput)) {
                        strcpy(errorMessage, "Failed to load image. Check if file exists and is a valid 24-bit BMP");
                        showErrorPopup = true;
                        errorTimer = 180;
                    } else {
                        selectedFilter=-1;
                    }
                }

                // Save button (hard-coded to output.bmp)
                if(GuiButton((Rectangle){rightPanel.x+8, rightPanel.y+150, rightPanel.width-16, 30}, "Save")){
                    if(!backend_save_image("output.bmp")) {
                        strcpy(errorMessage, "Failed to save image. Make sure an image is loaded first");
                        showErrorPopup = true;
                        errorTimer = 180;
                    } else {
                        strcpy(errorMessage, "Image saved successfully as output.bmp");
                        showErrorPopup = true;
                        errorTimer = 180;
                    }
                }

                // Allow clicking to focus textboxes
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mp = GetMousePosition();
                    if (CheckCollisionPointRec(mp, (Rectangle){leftPanel.x+8,leftPanel.y+70,leftPanel.width-16,30})) mainTextBoxActive=0;
                    if (CheckCollisionPointRec(mp, (Rectangle){leftPanel.x+60,leftPanel.y+185,leftPanel.width-68,30})) mainTextBoxActive=1;
                    if (CheckCollisionPointRec(mp, (Rectangle){leftPanel.x+60,leftPanel.y+225,leftPanel.width-68,30})) mainTextBoxActive=2;
                    if (CheckCollisionPointRec(mp, (Rectangle){rightPanel.x+8,rightPanel.y+70,rightPanel.width-16,30})) mainTextBoxActive=3;
                }


                // Logout
                if(GuiButton((Rectangle){screenWidth-120,screenHeight-50,100,40},"Logout")){
                    strcpy(loggedInUser,"");
                    currentScreen=LOGIN_SCREEN;
                    strcpy(username,""); strcpy(password,""); strcpy(confirmPassword,""); textBoxActive=0;
                }

                // ---------------- Center Panel ----------------
                DrawRectangleRec(centerPanel, Fade(DARKGRAY,0.02f));

                // Filters
                Rectangle filtersPanel = { centerPanel.x+10, centerPanel.y+10, (centerPanel.width-30)/2, centerPanel.height-20 };
                Rectangle filterView = { filtersPanel.x, filtersPanel.y+40, filtersPanel.width, filtersPanel.height-50 };
                Rectangle filterContent = {0,0,filterView.width-20,filterCount*40};
                DrawRectangleRec(filtersPanel, Fade(DARKGRAY,0.02f));
                DrawText("Filters", filtersPanel.x, filtersPanel.y, 24, DARKBLUE);
                GuiScrollPanel(filterView,NULL,filterContent,&filterScroll,NULL);
                BeginScissorMode(filterView.x,filterView.y,filterView.width,filterView.height);
                for(int i=0;i<filterCount;i++){
                    Rectangle item={filterView.x, filterView.y+i*40+(int)filterScroll.y, filterView.width,40};
                    if(GuiButton(item,filterOptions[i])){
                        // Check if image is loaded before applying filter
                        if(backend_get_width() == 0 || backend_get_height() == 0) {
                            strcpy(errorMessage, "Please load an image first");
                            showErrorPopup = true;
                            errorTimer = 180;
                        } else {
                            selectedFilter=i;
                            selectedTemplate=-1;
                            backend_apply_filter(selectedFilter);
                        }
                    }
                    if(i==selectedFilter) DrawRectangleRec(item, Fade(DARKGRAY,0.4f));
                }
                EndScissorMode();

                // Templates
                Rectangle templatesPanel = { centerPanel.x+20+(centerPanel.width-30)/2, centerPanel.y+10,
                                             (centerPanel.width-30)/2, centerPanel.height-20 };
                Rectangle tempView = { templatesPanel.x, templatesPanel.y+40, templatesPanel.width, templatesPanel.height-50 };
                Rectangle tempContent = {0,0,tempView.width-20,templateCount*40};
                DrawRectangleRec(templatesPanel, Fade(DARKGRAY,0.02f));
                DrawText("Templates", templatesPanel.x, templatesPanel.y, 24, DARKBLUE);
                GuiScrollPanel(tempView,NULL,tempContent,&tempScroll,NULL);
                BeginScissorMode(tempView.x,tempView.y,tempView.width,tempView.height);
                for(int i=0;i<templateCount;i++){
                    Rectangle item={tempView.x, tempView.y+i*40+(int)tempScroll.y, tempView.width,40};
                    if(GuiButton(item,templateOptions[i])){
                        selectedTemplate=i;
                        selectedFilter=-1;
                        backend_apply_template(selectedTemplate);
                    }
                    if(i==selectedTemplate) DrawRectangleRec(item, Fade(DARKGRAY,0.4f));
                }
                EndScissorMode();

            } break;
        }

        // ---------------- ERROR POP-UP ----------------------
        if(showErrorPopup) {
            Rectangle popupRect = {screenWidth/2 - 250, screenHeight/2 - 80, 500, 180};
            
            // Draw semi-transparent background overlay
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.5f));
            
            // Draw popup box
            DrawRectangleRec(popupRect, RAYWHITE);
            DrawRectangleLinesEx(popupRect, 3, RED);
            
            // Draw title
            DrawText("Error", popupRect.x + 20, popupRect.y + 10, 24, RED);
            DrawLine(popupRect.x + 10, popupRect.y + 40, popupRect.x + popupRect.width - 10, popupRect.y + 40, LIGHTGRAY);
            
            // Draw error message (simple display - Raylib handles long text)
            DrawText(errorMessage, popupRect.x + 20, popupRect.y + 55, 18, BLACK);
            
            // OK button
            Rectangle okButton = {popupRect.x + popupRect.width/2 - 50, popupRect.y + popupRect.height - 40, 100, 30};
            if(GuiButton(okButton, "OK")) {
                showErrorPopup = false;
                errorTimer = 0;
            }
        }

        EndDrawing();
    }

    backend_free(); // Free the image buffer on exit
    CloseWindow();
    return 0;
}
