# Folder Opener

## Overview

The Folder Opener is a C program designed to open folders specified in a text file. It supports multiple languages and provides verbose/silent modes for controlling output. It handles Windows, Linux, and macOS.

## Files

1. `folder_opener.c`: The main source file containing the code.
2. `folder_opener.languages`: The file storing language-specific messages.
3. `folder_opener.folders`: The file containing the list of folders to open.

## Dependencies

- Standard C libraries: `stdio.h`, `stdlib.h`, `string.h`, `sys/stat.h`
- Platform-specific headers:
  - Windows: `windows.h`, `shellapi.h`
  - Unix-like: `unistd.h`

## Functions and Logic

### `custom_getline`

- **Purpose:** A custom implementation of `getline` for reading lines from a file.
- **Logic:**
  - Allocates or resizes the line buffer.
  - Reads characters until EOF or newline is encountered.
  - Returns the length of the line read or -1 on error.

### `load_language`

- **Purpose:** Load language messages from a file based on the specified language ID.
- **Logic:**
  - Reads the language file line-by-line.
  - Checks if the current language ID matches the target.
  - Parses messages marked with `$message1`, `$message2`, etc.
  - Stores messages in a global `messages` array.
  - Returns 0 if all messages are loaded, otherwise returns 1.

### `folder_exists`

- **Purpose:** Check if a specified folder exists.
- **Logic:**
  - Uses `stat` to retrieve file information.
  - Returns true if the path exists and is a directory.

### `sanitize_input`

- **Purpose:** Validates the input to prevent command injection.
- **Logic:**
  - Checks for special characters (`&`, `?`, `*`, etc.) in the input.
  - Returns true if any unsafe character is found.

### `open_folder`

- **Purpose:** Open the specified folder.
- **Logic:**
  - Validates input using `sanitize_input`.
  - Checks if the folder exists using `folder_exists`.
  - Depending on the platform:
    - Windows: Uses `ShellExecuteEx`.
    - macOS: Uses `/usr/bin/open`.
    - Linux: Uses `xdg-open`.
  - Prints appropriate error messages using loaded language strings.

### `main`

- **Purpose:** Entry point of the program.
- **Logic:**
  - Sets UTF-8 output on Windows.
  - Parses command-line arguments for language and verbosity.
  - Loads language messages.
  - Checks other command-line options, printing unknown options.
  - Reads the folders file line-by-line.
  - Opens each folder by calling `open_folder`.
  - Frees allocated message memory.

## Compilation

Compile the code using a C compiler. Example for GCC:

```sh
gcc -o folder_opener folder_opener.c
```

## Usage

Run the program specifying the language:

```sh
./folder_opener -language HR
```

Or, if no language is specified, it defaults to English.
