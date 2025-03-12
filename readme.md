# Lively Shuffle

## What is Lively Shuffle?
Lively Shuffle enhances the **Lively Wallpaper** experience by allowing you to **randomize your wallpapers**, a feature that Lively does not natively support. From the system tray menu, it also provides:
- **Switching on demand** to the next random wallpaper.
- **Customizable shuffle delay** for better control over your wallpaper transitions.

---

## Requirements
To build and run Lively Shuffle, you need:
- **Windows 10 or 11**
- **MinGW-w64** (for compiling on Windows)
- **Lively Wallpaper** (Download: [Lively Wallpaper](https://rocksdanister.github.io/lively/))
- **LivelyCU** (Command Line Tool for Lively)
- **Lively PATH variable** (Recommended for easy access)

---

## Compilation Instructions
Open a terminal in the project directory and run the following commands:

```sh
windres dialog.rc -o dialog.o
g++ -o lively_shuffle.exe lively_shuffle.cpp dialog.o -std=c++17 -mwindows -lstdc++fs
```

This will generate `lively_shuffle.exe`, which you can use to shuffle wallpapers in Lively.

---

## Lively Shuffle Setup
### 1. Configure Lively Paths
1. Navigate to the installation folder:
   ```
   C:\Program Files (x86)\Lively Shuffle
   ```
   _(or wherever you installed Lively Shuffle)_

2. Open **`lively_paths.txt`** with a text editor.

3. Set the paths for `livelycu.exe` and your wallpaper folder:
   ```
   livelycu_path = "C:\path\to\livelycu.exe"
   wallpaper_path = "C:\path\to\wallpaper\folder"
   ```

### 2. Add Wallpapers
Place your wallpapers inside the folder specified in `wallpaper_path`.

### 3. Set Shuffle Delay
Right-click the **Lively Shuffle** system tray icon and set your desired shuffle delay.

### 4. Enjoy!
Your wallpapers will now shuffle randomly based on your settings. 🎉

---

## Need Help?
If you encounter any issues, check:
- **Lively Wallpaper Documentation**: [Lively Docs](https://rocksdanister.github.io/lively/)
- **MinGW-w64 Setup**: [MinGW-w64 Installation Guide](https://www.mingw-w64.org/)
- Ensure your **Lively PATH** is correctly set for easier access.

## Author
[**valekll**](https://github.com/valekll)
