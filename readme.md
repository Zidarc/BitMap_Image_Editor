# BMP Image Editor (PF Lab Project)

A lightweight GUI tool for opening, editing, and saving BMP images. This project was built in C as a Programming Fundamentals (PF) lab project at FAST NU KHI using **raylib** and **raygui** for the interface.

---

## 📌 Overview

The BMP Image Editor is designed for beginners who want a simple, focused tool to perform common image edits on **.bmp** files without the complexity of full-scale editors like Photoshop or GIMP. The app provides a straightforward workflow: load a BMP file, apply edits, preview results, and save the updated image.

---

## ✅ Features

### Core Image Tools
- Open and save **.bmp** images
- Grayscale filter
- Brightness adjustment
- Color inversion
- Crop
- Resize
- Horizontal and vertical flipping *(optional)*

### User Experience
- Simple GUI built with raylib + raygui
- Clear controls for each operation

### Login System
- Create/login users
- Track edited image paths per user

### Social Media Templates
- Quick resizing for popular formats (Instagram profile, Instagram post, Facebook post, etc.)

---

## 🖼️ Screenshots (Placeholders)

- **[Main Editor Screenshot goes here]**
- **[Edit Result Screenshot goes here]**

---

## 🧰 Tech Stack

- **Language:** C
- **GUI:** raylib + raygui
- **Platform:** Windows

---

## 🚀 Build & Run

### Prerequisites
- GCC (MinGW on Windows)
- raylib installed at `C:/raylib/raylib/src` (adjust if your path differs)

### Build Command

```bash
gcc -o bmt.exe backend.c bitmapgui.c login.c -I"C:/raylib/raylib/src" -L"C:/raylib/raylib/src" -lraylib -lopengl32 -lgdi32 -lwinmm -luser32
```

### Run

```bash
bmt.exe
```

---

## 🧭 Usage

1. Launch the app.
2. Log in or create a user.
3. Open a `.bmp` file.
4. Apply edits (grayscale, brightness, crop, resize, etc.).
5. Save the modified image.

---

## 📂 Repository Structure

- `backend.c` / `backend.h` — core image processing functions
- `bitmapgui.c` — GUI rendering and interaction logic
- `login.c` / `login.h` — user login and history tracking
- `bmp.h` — BMP format definitions and helpers
- `Images/` — image assets (add screenshots here)
- `Testing files/` — sample input images
- `runningcommand.txt` — build command reference

---

## ⚠️ Limitations

- Only BMP format is supported.
- Windows-focused build steps.

---

## 🛠️ Troubleshooting

- **App won’t compile:** Verify raylib include and library paths in the build command.
- **Missing DLL errors:** Ensure raylib dependencies are available in your PATH.
- **Images don’t load:** Confirm the file is a valid `.bmp`.

---

## 🙌 Credits

Developed as a PF Lab project at FAST NU KHI.

---

## 📎 Notes

If you add screenshots, place them in the `Images/` folder and replace the placeholders above with real image links or markdown image tags.
