# Database Setup for Bayou Bonanza Development

This document outlines how to set up the SQLite3 database for local development of the Bayou Bonanza server.

## Prerequisites

You will need to have SQLite3 development libraries installed on your system.

*   **For Debian/Ubuntu-based systems:**
    ```bash
    sudo apt-get update
    sudo apt-get install libsqlite3-dev
    ```
*   **For other systems:**
    Please refer to the official SQLite documentation for instructions on installing the development libraries.

## Database Initialization

The Bayou Bonanza server is configured to automatically initialize the database when it first starts.

1.  **Database File:** A file named `bayou_bonanza.db` will be created in the same directory where the server executable is run.
2.  **`users` Table:** A table named `users` will be automatically created with the following schema:
    *   `username` (TEXT, PRIMARY KEY, NOT NULL): The player's unique username.
    *   `rating` (INTEGER, NOT NULL, DEFAULT 1000): The player's Elo rating, defaulting to 1000 for new players.

No manual schema creation is required.

## Inspecting the Database (Optional)

If you need to inspect the contents of the database directly, you can use the `sqlite3` command-line tool.

1.  **Open the database file:**
    ```bash
    sqlite3 bayou_bonanza.db
    ```
2.  **Useful SQLite commands:**
    *   `.tables`: List all tables in the database.
    *   `.schema users`: Show the schema for the `users` table.
    *   `SELECT * FROM users;`: View all records in the `users` table.
    *   `.quit`: Exit the sqlite3 CLI.

This setup ensures that developers can get the server running with its database dependencies with minimal manual intervention.
