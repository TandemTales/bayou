# Database Setup for Bayou Bonanza Development

This document outlines how to set up the SQLite3 database for local development of the Bayou Bonanza server.

## Prerequisites

### For Linux/Ubuntu-based systems:
You will need to have SQLite3 development libraries installed on your system.

```bash
sudo apt-get update
sudo apt-get install libsqlite3-dev
```

### For Windows Development:
**No manual installation required!** The project is configured to automatically download and build SQLite3 using CMake's FetchContent feature.

## Windows Setup (Automatic)

The CMakeLists.txt file has been configured to automatically handle SQLite3 on Windows:

1. **Automatic Download**: If SQLite3 is not found on the system, CMake will automatically download the SQLite3 amalgamation from a reliable GitHub repository.

2. **Automatic Build**: The downloaded SQLite3 source will be compiled as a static library and linked to the BayouBonanzaServer.

3. **No Manual Steps**: Simply run the standard CMake build process:

```powershell
# Create and navigate to build directory
mkdir build
cd build

# Configure the project (this will download SQLite3 automatically)
cmake ..

# Build the server
cmake --build . --target BayouBonanzaServer
```

## Database Initialization

The Bayou Bonanza server is configured to automatically initialize the database when it first starts.

1. **Database Location:** The database file `bayou_bonanza.db` is always created in the **project root directory** (`C:\dev\bayou\`), regardless of where the server executable is run from. This ensures consistent data location across different execution environments.

2. **Database File:** A file named `bayou_bonanza.db` will be created in the project root directory.

3. **`users` Table:** A table named `users` will be automatically created with the following schema:
   * `username` (TEXT, PRIMARY KEY, NOT NULL): The player's unique username.
   * `rating` (INTEGER, NOT NULL, DEFAULT 1000): The player's Elo rating, defaulting to 1000 for new players.

4. **`collections` Table:** Stores each player's owned cards as a serialized list.

5. **`decks` Table:** Stores the player's current deck in serialized form. Only one deck per user is supported.

No manual schema creation is required.

## Troubleshooting

### Build Errors Related to SQLite3

If you encounter the error:
```
error C1083: Cannot open include file: 'sqlite3.h': No such file or directory
```

This means the automatic SQLite3 download failed. Try:

1. **Clean and Regenerate**: Delete the build directory and regenerate:
   ```powershell
   Remove-Item -Recurse -Force build
   mkdir build
   cd build
   cmake ..
   ```

2. **Check Internet Connection**: Ensure you have internet access for CMake to download SQLite3 from GitHub.

3. **Manual Alternative**: If automatic download continues to fail, you can manually download SQLite3:
   - Download the SQLite3 amalgamation from https://github.com/azadkuh/sqlite-amalgamation
   - Extract to a local directory
   - Modify CMakeLists.txt to point to your local copy

### Visual Studio Lock Issues

If you get file access errors when trying to clean the build directory:
- Close Visual Studio completely

## Inspecting the Database (Optional)

If you need to inspect the contents of the database directly, you can use the `sqlite3` command-line tool.

### Installing SQLite3 Command-Line Tool (Windows)

If you don't have the SQLite3 command-line tool installed, you can install it using Windows Package Manager:

```powershell
# Install SQLite3 using winget
winget install SQLite.SQLite

# Restart your PowerShell session or open a new terminal window
# The PATH environment variable will be updated automatically
```

**Note:** After installation, you may need to restart your terminal or open a new PowerShell window for the `sqlite3` command to be recognized.

If the command is still not recognized after restarting, you can manually add it to your current session:
```powershell
$env:PATH += ";$env:LOCALAPPDATA\Microsoft\WinGet\Packages\SQLite.SQLite_Microsoft.Winget.Source_8wekyb3d8bbwe"
```

### Using SQLite3 to Inspect the Database

1. **Open the database file:**
   ```bash
   sqlite3 bayou_bonanza.db
   ```
2. **Useful SQLite commands:**
   * `.tables`: List all tables in the database.
   * `.schema users`: Show the schema for the `users` table.
   * `SELECT * FROM users;`: View all records in the `users` table.
   * `.quit`: Exit the sqlite3 CLI.

**Example session:**
```powershell
# List tables
sqlite3 bayou_bonanza.db ".tables"

# Show users table schema
sqlite3 bayou_bonanza.db ".schema users"

# View all users (if any)
sqlite3 bayou_bonanza.db "SELECT * FROM users;"
```

## Technical Details

The Windows setup uses CMake's FetchContent feature to:
- Download SQLite3 amalgamation from https://github.com/azadkuh/sqlite-amalgamation
- Create a static library target (`sqlite3_lib`)
- Link it to the BayouBonanzaServer target
- Provide the necessary include directories

This approach ensures that developers can get the server running with its database dependencies with minimal manual intervention, regardless of their Windows environment setup.
