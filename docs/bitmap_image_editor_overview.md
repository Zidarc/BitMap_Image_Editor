# BMP Image Editor – Codebase Overview

## 1. Purpose and Scope
This document explains the structure of the BMP Image Editor project located in `C:\Users\Hp Probook\Desktop\BitMap_Image_Editor`. It covers every source file, the functions they expose, how components interact, and notable behaviors or constraints so new contributors can navigate the code quickly.

## 2. High-Level Architecture
- **Domain structs** live in `bmp.h` and define the packed Windows BMP headers (`BITMAPFILEHEADER`, `BITMAPINFOHEADER`) plus the `RGBTRIPLE` pixel structure and Win32-style typedefs. Every other module depends on these definitions when loading or writing 24-bit BMP files.
- **Image processing algorithms** are implemented in `filters.c` with declarations in `filters.h`. All filters operate on dynamically allocated 2D RGB matrices and are reused by the console and GUI front-ends.
- **Console workflow** (`main.c`) is a self-contained CLI program. It loads an image from disk, prompts the user for a filter selection (and any required parameters), calls the relevant filter, updates BMP headers, and writes the edited bitmap back to disk.
- **GUI workflow** (`bitmapgui.c` + `backend.c`) uses Raylib/Raygui. `bitmapgui.c` handles windowing, the login/signup flow, and the filter/template panels. `backend.c` maintains the currently loaded BMP in memory and exposes helper functions so the GUI can load, save, and apply specific filters without duplicating I/O code.
- **Authentication** (`login.c` / `login.h`) provides simple username/password storage in `users.txt` using XOR+hex encoding to obscure passwords. Both CLI and GUI can reuse these functions.
- **Supporting assets** include sample BMPs under `Images/`, previous GUI experiments (`raylibtest.c`), placeholder generators (`BMP_Maker.c`), and vendor headers (`raylib.h`, `raygui.h`). `runningcommand.txt` documents the gcc build command used on Windows.

The project currently targets 24-bit, uncompressed BMP images with canonical 54-byte headers. Attempts to load incompatible files abort gracefully.

## 3. Module Reference

### 3.1 `bmp.h`
- Provides packed structure definitions for BMP metadata and pixels, ensuring byte-accurate serialization.
- Offers aliases (`BYTE`, `WORD`, `DWORD`, `LONG`) matching Win32 definitions. These are used widely for clarity when dealing with BMP headers.

### 3.2 `filters.h`
- Declares every filter function plus rotation/resize helpers. All image-manipulation routines accept height/width and a pointer to a 2D array of `RGBTRIPLE`.
- Summary of declared functions:
  - `grayscale`, `sepia`, `reflect`, `blur`, `edges` (declared, not implemented), `adjust_brightness`, `adjust_contrast`, `invert_colors`, `pixelate`, `vignette`, `sharpen`, `gaussian_blur`, `emboss`, `add_border`
  - Rotation/resizing helpers (`rotate_90`, `rotate_180`, `rotate_270`, `resize`) take dimension references so callers can update header metadata.

### 3.3 `filters.c`
Implements nearly all filters plus helper utilities:

| Function | Purpose | Implementation Notes |
| --- | --- | --- |
| `static inline int clamp(int value)` | Keep channel values in `[0, 255]` | Uses `fmax/fmin`; reused everywhere brightness/contrast adjustments occur. |
| `grayscale` | Average a pixel’s RGB components and write the same value to all channels | Uses `round` for even distribution. |
| `sepia` | Apply the standard sepia matrix | Each channel computed from original RGB values then clamped. |
| `reflect` | Horizontal mirror effect | Swaps pixels around the vertical center per row. |
| `blur` | Box blur with 3×3 kernel | Allocates a temporary image buffer, averages neighbor pixels, then copies back. |
| `invert_colors` | Subtract each channel from 255 | In-place. |
| `adjust_brightness` | Add constant brightness to all channels | Uses `clamp` to avoid overflow. |
| `adjust_contrast` | Multiply difference from midpoint (128) by factor | Factor is user-provided float. |
| `pixelate` | Divides the image into square blocks and replaces each block with its average color | Block size is caller-controlled. |
| `vignette` | Darkens pixels farther from the center using a radial falloff | Computes distance from image center for each pixel. |
| `sharpen` | 3×3 sharpening convolution | Uses kernel `[[0,-1,0],[-1,5,-1],[0,-1,0]]`. |
| `gaussian_blur` | 5×5 Gaussian blur | Pre-normalized kernel weights sum to 1.0. |
| `emboss` | Emboss-style convolution | Kernel `{{-2,-1,0},{-1,1,1},{0,1,2}}`. |
| `add_border` | Paints a border of configurable width and color | Overwrites pixels where `i`/`j` lie inside the border region. |
| `rotate_90` / `rotate_270` | Allocate new buffers, remap pixels, update height/width/padding references | Caller must update BMP headers after rotation. |
| `rotate_180` | In-place 180° rotation | Swaps pixel positions, handles middle row for odd heights. |
| `resize` | Nearest-neighbor resizing to arbitrary dimensions | Updates headers (`bfSize`, `biSizeImage`) to remain consistent. |

> **Gaps:** `edges` is declared in the header but missing here, so linking will fail if it is referenced. Adding the implementation or removing the declaration is necessary before exposing edge detection in the UI.

### 3.4 `main.c`
Implements the console tool:
1. Hard-coded paths: reads `./Images/desert.bmp` and writes `./images/outsample1.bmp` (note the lowercase `images`). Consider making these arguments or validating folder case on Windows.
2. Validates the BMP header to ensure it is a 24-bit uncompressed bitmap with a 54-byte data offset.
3. Allocates a contiguous 2D array: `RGBTRIPLE (*pixelArray)[width] = calloc(height, width * sizeof(RGBTRIPLE));`. This layout provides cache-friendly access during filtering.
4. Reads each scanline and skips padding bytes (`(4 - ((width * 3) % 4)) % 4`), populating the array.
5. Presents an 18-option menu. For filters that need parameters (brightness, contrast, block size, border color, resize dimensions), it prompts the user and validates ranges.
6. Calls the appropriate filter function. Rotations and resize functions receive references to header fields and padding so they can modify image metadata.
7. After manipulation, writes updated headers and pixel data to the output file, reinserting padding at the end of each scanline, then frees memory.

This entry point is useful for testing filters without the GUI; however, it duplicates some logic from `backend.c`.

### 3.5 `backend.c` & `backend.h`
`backend.c` provides stateful helpers used by the GUI:
- **Global state:** static `BITMAPFILEHEADER fileheader`, `BITMAPINFOHEADER infoheader`, `RGBTRIPLE **pixelArray`, and cached width/height.
- `backend_load_image(const char *filepath)`: Opens the file, validates headers, allocates `imgHeight` rows of `imgWidth` pixels, reads the bitmap while skipping padding, and stores the values in global state. Returns `1` on success.
- `backend_save_image(const char *outfile)`: Recomputes padding and `biSizeImage`, updates `bfSize`, then writes headers and pixel rows to disk.
- `backend_apply_filter(int filterID)`: Switch statement currently supports only grayscale (`case 5`) and invert (`case 6`). Other cases are placeholders until the GUI can collect user input (e.g., brightness intensity). The ID mapping mirrors `filterOptions` in `bitmapgui.c`.
- `backend_apply_template(int templateID)`: Stub for future presets (currently does nothing).
- `backend_get_width` / `backend_get_height`: Accessors for the GUI (e.g., to scale previews).
- `backend_free`: Declared but **not implemented**, so the current GUI never releases `pixelArray`. Implementing this is critical to avoid leaks when loading multiple images in one session.

### 3.6 `bitmapgui.c`
- Uses Raylib to render a window with three main screens (`LOGIN_SCREEN`, `SIGNUP_SCREEN`, `MAIN_SCREEN`).
- Manages text boxes for username/password, buttons for login/signup, and transitions between screens using `GameScreen` enum.
- Upon successful login, `currentScreen` switches to `MAIN_SCREEN`, which features three panels:
  - **Left panel:** reserved for edit history (not implemented yet).
  - **Center panel:** two scrollable lists, one for filters and one for preset templates. Clicking an item highlights it and stores the selection index; actual backend calls are currently commented out except when an image is loaded.
  - **Right panel:** file path input and “Load” button. When clicked, it passes the path to `backend_load_image`. If loading succeeds and a filter is selected, the GUI immediately calls `backend_apply_filter(selectedFilter)` and then saves to `output.bmp`.
- Relies on `login.c` for credential checks (calls `login` / `signup` directly).
- To extend: connect each filter button to parameter dialogs (e.g., slider for brightness) and call `backend_apply_filter` or new backend functions with those parameters; implement template selection using `backend_apply_template`.

### 3.7 `login.c` / `login.h`
- `signup`: opens `users.txt`, checks for existing usernames, encrypts the password with XOR key `7` followed by hex encoding, and appends `username encryptedPassword` to the file.
- `login`: scans `users.txt`, decrypts stored passwords on the fly, and compares them with the provided password.
- `encrypt`/`decrypt`: helper routines implementing XOR + hex encoding round-trip.
- Return codes are defined in `login.h` (`LOGIN_OK`, `LOGIN_USERNAME_EXISTS`, etc.).
- Note: this scheme is for demo purposes only—passwords are easily reversible. Replace with salted hashing for production.

### 3.8 Additional Files
- `raylibtest.c`: Earlier prototype demonstrating screen transitions, dropdowns, and basic GUI controls within Raylib/Raygui. Helpful as a sandbox but not part of the main build.
- `BMP_Maker.c`: Placeholder comment indicating future functionality (“Can generate Images”).
- `raylib.h` / `raygui.h`: Bundled library headers.
- `runningcommand.txt`: Build command showing how to compile the GUI with Raylib on Windows:  
  `gcc -o bmt.exe backend.c bitmapgui.c login.c filters.c -I"C:/raylib/raylib/src" -L"C:/raylib/raylib/src" -lraylib -lopengl32 -lgdi32 -lwinmm -luser32`.
- `Images/`: Sample BMP files for development and testing. The CLI currently references `./Images/desert.bmp`.
- `users.txt`: Runtime-generated credential file (not version-controlled ideally). Contains `username encryptedPassword` lines.
- `output.bmp`: Result file produced by either the CLI or GUI flows when saving edits.

## 4. Data Flow & Interactions
1. **Loading a BMP**
   - CLI: `main.c` directly reads the file, validates, and fills a contiguous pixel array.
   - GUI: `bitmapgui.c` → `backend_load_image` (which handles allocation and reading). Pixel data is stored globally for repeated filter applications.
2. **Applying Filters**
   - CLI: `main.c` calls the selected filter function directly with `height`, `width`, and `pixelArray`.
   - GUI: `bitmapgui.c` keeps track of `selectedFilter` and triggers `backend_apply_filter`, which dispatches to a subset of `filters.c` routines.
3. **Saving a BMP**
   - CLI: After manipulation, `main.c` writes headers + pixel data using the same stride logic used for reading.
   - GUI: `backend_save_image` handles serialization, invoked by the “Load” button after applying the chosen filter or could be triggered elsewhere in the GUI.
4. **Authentication**
   - GUI login/signup screens call `login()`/`signup()` from `login.c`. These functions manage `users.txt` and return status codes for UI feedback.

## 5. Notable Constraints & Open Items
- Only 24-bit uncompressed BMPs with 54-byte offsets are supported. Attempting to load compressed or color-indexed BMPs results in immediate failure.
- `filters.c` lacks the declared `edges` implementation and the GUI backend currently exposes only grayscale and invert filters; other filters require user-input plumbing and backend updates.
- Memory management in `backend.c` needs a `backend_free` implementation to release `pixelArray` and associated rows when the GUI closes or loads a new image.
- The CLI and GUI duplicate BMP parsing logic. Consider extracting shared utilities to avoid divergence.
- Password storage is not secure; use salted hashes and file permission checks before shipping beyond demos.
- GUI template functionality and edit history panes are placeholders.

## 6. Suggested Enhancements
1. **Complete Filter Coverage**: Implement `edges` in `filters.c` (e.g., Sobel operator) or remove the menu option. Expand `backend_apply_filter` to call additional filters, adding GUI controls for parameterized effects.
2. **Shared BMP Loader/Saver**: Refactor load/save routines into a dedicated module (`image_io.c`) to reduce duplication between CLI and backend.
3. **Resource Cleanup**: Implement `backend_free` and call it when closing the GUI or before loading a different image.
4. **Improved CLI UX**: Accept input/output paths and filter selections via command-line flags to script batch conversions.
5. **GUI Feedback**: Display a preview of the currently loaded image (Raylib can load textures from memory) and show toast messages for success/failure outcomes.
6. **Security**: Replace XOR-based password encoding with a proper hashing algorithm (e.g., bcrypt or PBKDF2) and move credentials outside the repo folder.

## 7. Generating This Document
To regenerate or update this documentation:
1. Edit `docs/bitmap_image_editor_overview.md`.
2. Use the Python script (see project history) or a tool like `pandoc` to produce a PDF if needed.

---
For clarifications or future improvements, add comments to this document or create issues describing required changes so the documentation remains accurate as the project evolves.

