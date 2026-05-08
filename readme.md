# BMP Image Editor (PF Lab Project)

A lightweight GUI tool for opening, editing, and saving BMP images. This project was built in C as a Programming Fundamentals (PF) lab project at FAST NU KHI using **raylib** and **raygui** for the interface.

---

## 📌 Project Overview

The BMP Image Editor focuses on beginner-friendly image manipulation features without the complexity of full-scale editors like Photoshop or GIMP. It provides a simple workflow to load a BMP file, apply edits, preview changes, and save the output.

---

## ✅ Key Features

- Open and save **.bmp** images
- Grayscale filter
- Brightness adjustment
- Color inversion
- Crop and resize
- Horizontal/vertical flipping *(optional)*
- Simple GUI controls for all operations

### 🔐 Login System

- Create/login users
- Track edited image paths per user

### 📱 Social Media Templates

- Quick resizing for popular formats (Instagram profile, Instagram post, Facebook post, etc.)

---

## 🖼️ Screenshots

> **Placeholder:** Add a screenshot of the main editor window.

![Main Editor Screenshot](Images/placeholder-main-editor.png)

> **Placeholder:** Add a screenshot of an edit result.

![Edit Result Screenshot](Images/placeholder-edit-result.png)

---

## 🧰 Tools & Technologies

- **Language:** C
- **GUI:** raylib + raygui
- **OS:** Windows

---

## 🚀 Build & Run

> The project is compiled with GCC and linked against raylib.

```bash
gcc -o bmt.exe backend.c bitmapgui.c login.c -I"C:/raylib/raylib/src" -L"C:/raylib/raylib/src" -lraylib -lopengl32 -lgdi32 -lwinmm -luser32
```

---

## 📂 Repository Structure

- `backend.c` / `backend.h` — core image processing functions
- `bitmapgui.c` — GUI rendering and interaction logic
- `login.c` / `login.h` — user login and history tracking
- `Images/` — image assets (add screenshots here)
- `Testing files/` — sample inputs

---

## 📎 Notes

- Only BMP format is supported.
- Use the `Images/` folder to store screenshots referenced above.
