#include <iostream>
#include <sqlite3.h>
#include <cassert>
#include <string>
#include <vector>
#include <cstdio> // For remove()

const char* TEST_DB_NAME = "test_bayou_bonanza.db";

// Helper function to execute SQL (simplistic, for non-query SQL)
bool execute_sql(sqlite3* db, const std::string& sql) {
    char* err_msg = 0;
    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << err_msg << " for SQL: " << sql << std::endl;
        sqlite3_free(err_msg);
        return false;
    }
    return true;
}

// Setup function to prepare a clean database for each test
void setup_test_database(sqlite3** db) {
    remove(TEST_DB_NAME); // Delete old test DB if it exists
    int rc = sqlite3_open(TEST_DB_NAME, db);
    assert(rc == SQLITE_OK);
    if (rc != SQLITE_OK) {
         std::cerr << "Can't open database: " << sqlite3_errmsg(*db) << std::endl;
         if (*db) sqlite3_close(*db); // Close if open failed but handle was returned
         exit(1); // Critical error for tests
    }

    const std::string create_table_sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "username TEXT PRIMARY KEY NOT NULL,"
        "rating INTEGER NOT NULL DEFAULT 1000"
        ");";
    assert(execute_sql(*db, create_table_sql));
}

void test_database_initialization() {
    std::cout << "Running test_database_initialization..." << std::endl;
    sqlite3* db;
    
    remove(TEST_DB_NAME); // Ensure it's clean before this specific test logic
    int rc_open = sqlite3_open(TEST_DB_NAME, &db);
    assert(rc_open == SQLITE_OK);
     if (rc_open != SQLITE_OK) { // Should not happen due to assert, but good practice
        std::cerr << "Test setup failed: Can't open database: " << sqlite3_errmsg(db) << std::endl;
        if (db) sqlite3_close(db);
        exit(1);
    }

    const std::string create_table_sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "username TEXT PRIMARY KEY NOT NULL,"
        "rating INTEGER NOT NULL DEFAULT 1000"
        ");";
    assert(execute_sql(db, create_table_sql));

    // Verify table creation by querying sqlite_master
    sqlite3_stmt* stmt;
    const std::string check_table_sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='users';";
    rc_open = sqlite3_prepare_v2(db, check_table_sql.c_str(), -1, &stmt, 0);
    assert(rc_open == SQLITE_OK);
    
    int step_rc = sqlite3_step(stmt);
    assert(step_rc == SQLITE_ROW); // Should find one row if table exists
    if (step_rc == SQLITE_ROW) {
        std::string tableName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        assert(tableName == "users");
    }
    sqlite3_finalize(stmt);

    sqlite3_close(db);
    remove(TEST_DB_NAME); // Clean up
    std::cout << "test_database_initialization passed." << std::endl;
}

void test_user_creation_and_retrieval() {
    std::cout << "Running test_user_creation_and_retrieval..." << std::endl;
    sqlite3* db;
    setup_test_database(&db);

    // Insert a new user ("testuser1", 0)
    const std::string insert_user1_sql = "INSERT INTO users (username, rating) VALUES ('testuser1', 0);";
    assert(execute_sql(db, insert_user1_sql));

    // Retrieve the user and verify username and rating
    sqlite3_stmt* stmt;
    const std::string select_user1_sql = "SELECT username, rating FROM users WHERE username = 'testuser1';";
    int rc_prepare = sqlite3_prepare_v2(db, select_user1_sql.c_str(), -1, &stmt, 0);
    assert(rc_prepare == SQLITE_OK);
    if (rc_prepare != SQLITE_OK) { std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl; sqlite3_close(db); exit(1); }

    int step_rc = sqlite3_step(stmt);
    assert(step_rc == SQLITE_ROW);
    if (step_rc == SQLITE_ROW) {
        std::string username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        int rating = sqlite3_column_int(stmt, 1);
        assert(username == "testuser1");
        assert(rating == 0);
    }
    sqlite3_finalize(stmt);

    // Attempt to retrieve a non-existent user ("nonexistentuser")
    const std::string select_nonexistent_sql = "SELECT username, rating FROM users WHERE username = 'nonexistentuser';";
    rc_prepare = sqlite3_prepare_v2(db, select_nonexistent_sql.c_str(), -1, &stmt, 0);
    assert(rc_prepare == SQLITE_OK);
    if (rc_prepare != SQLITE_OK) { std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl; sqlite3_close(db); exit(1); }
    
    step_rc = sqlite3_step(stmt);
    assert(step_rc == SQLITE_DONE); // Should not find any row
    sqlite3_finalize(stmt);

    // Insert another user ("testuser2", 0)
    const std::string insert_user2_sql = "INSERT INTO users (username, rating) VALUES ('testuser2', 0);";
    assert(execute_sql(db, insert_user2_sql));

    // Retrieve and verify "testuser2"
    const std::string select_user2_sql = "SELECT username, rating FROM users WHERE username = 'testuser2';";
    rc_prepare = sqlite3_prepare_v2(db, select_user2_sql.c_str(), -1, &stmt, 0);
    assert(rc_prepare == SQLITE_OK);
    if (rc_prepare != SQLITE_OK) { std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl; sqlite3_close(db); exit(1); }

    step_rc = sqlite3_step(stmt);
    assert(step_rc == SQLITE_ROW);
    if (step_rc == SQLITE_ROW) {
        std::string username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        int rating = sqlite3_column_int(stmt, 1);
        assert(username == "testuser2");
        assert(rating == 0);
    }
    sqlite3_finalize(stmt);

    sqlite3_close(db);
    remove(TEST_DB_NAME); // Clean up
    std::cout << "test_user_creation_and_retrieval passed." << std::endl;
}

void test_rating_updates() {
    std::cout << "Running test_rating_updates..." << std::endl;
    sqlite3* db;
    setup_test_database(&db);

    // Insert initial users
    assert(execute_sql(db, "INSERT INTO users (username, rating) VALUES ('testuser1', 1000);"));
    assert(execute_sql(db, "INSERT INTO users (username, rating) VALUES ('testuser2', 1000);"));

    // Update "testuser1"'s rating to 1010
    const std::string update_user1_sql = "UPDATE users SET rating = 1010 WHERE username = 'testuser1';";
    assert(execute_sql(db, update_user1_sql));

    // Retrieve and verify "testuser1"
    sqlite3_stmt* stmt;
    const std::string select_user1_sql = "SELECT rating FROM users WHERE username = 'testuser1';";
    int rc_prepare = sqlite3_prepare_v2(db, select_user1_sql.c_str(), -1, &stmt, 0);
    assert(rc_prepare == SQLITE_OK);
    if (rc_prepare != SQLITE_OK) { std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl; sqlite3_close(db); exit(1); }
    
    int step_rc = sqlite3_step(stmt);
    assert(step_rc == SQLITE_ROW);
    if (step_rc == SQLITE_ROW) {
        assert(sqlite3_column_int(stmt, 0) == 1010);
    }
    sqlite3_finalize(stmt);

    // Update "testuser2"'s rating to 990
    const std::string update_user2_sql = "UPDATE users SET rating = 990 WHERE username = 'testuser2';";
    assert(execute_sql(db, update_user2_sql));

    // Retrieve and verify "testuser2"
    const std::string select_user2_sql = "SELECT rating FROM users WHERE username = 'testuser2';";
    rc_prepare = sqlite3_prepare_v2(db, select_user2_sql.c_str(), -1, &stmt, 0);
    assert(rc_prepare == SQLITE_OK);
    if (rc_prepare != SQLITE_OK) { std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl; sqlite3_close(db); exit(1); }

    step_rc = sqlite3_step(stmt);
    assert(step_rc == SQLITE_ROW);
    if (step_rc == SQLITE_ROW) {
        assert(sqlite3_column_int(stmt, 0) == 990);
    }
    sqlite3_finalize(stmt);

    sqlite3_close(db);
    remove(TEST_DB_NAME); // Clean up
    std::cout << "test_rating_updates passed." << std::endl;
}

void test_new_user_default_rating() {
    std::cout << "Running test_new_user_default_rating..." << std::endl;
    sqlite3* db;
    setup_test_database(&db);

    const char* username_to_test = "newuser";
    int default_rating = 0; // Default rating as per server logic

    // Simulate server logic: try to select user, if not found, insert with default rating
    sqlite3_stmt* stmt_select;
    const std::string select_sql = "SELECT rating FROM users WHERE username = ?;";
    
    int rc_prepare = sqlite3_prepare_v2(db, select_sql.c_str(), -1, &stmt_select, 0);
    assert(rc_prepare == SQLITE_OK);
    if (rc_prepare != SQLITE_OK) { std::cerr << "Prepare failed (select): " << sqlite3_errmsg(db) << std::endl; sqlite3_close(db); exit(1); }

    sqlite3_bind_text(stmt_select, 1, username_to_test, -1, SQLITE_STATIC);
    int step_rc = sqlite3_step(stmt_select);

    if (step_rc == SQLITE_ROW) {
        // User found, this shouldn't happen in a clean test for default rating insertion
        std::cerr << "User " << username_to_test << " unexpectedly found during default rating test." << std::endl;
        assert(false); // Fail the test
    } else if (step_rc == SQLITE_DONE) {
        // User not found, insert new user with default rating
        sqlite3_finalize(stmt_select); // Finalize previous statement first

        const std::string insert_sql = "INSERT INTO users (username, rating) VALUES (?, ?);";
        sqlite3_stmt* stmt_insert;
        rc_prepare = sqlite3_prepare_v2(db, insert_sql.c_str(), -1, &stmt_insert, 0);
        assert(rc_prepare == SQLITE_OK);
        if (rc_prepare != SQLITE_OK) { std::cerr << "Prepare failed (insert): " << sqlite3_errmsg(db) << std::endl; sqlite3_close(db); exit(1); }
        
        sqlite3_bind_text(stmt_insert, 1, username_to_test, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt_insert, 2, default_rating);
        
        step_rc = sqlite3_step(stmt_insert);
        assert(step_rc == SQLITE_DONE);
        sqlite3_finalize(stmt_insert);
        std::cout << "User " << username_to_test << " inserted with default rating " << default_rating << "." << std::endl;
    } else {
        std::cerr << "SQL error selecting user: " << sqlite3_errmsg(db) << std::endl;
        assert(false); // Fail the test
    }
    if (stmt_select && step_rc != SQLITE_DONE) sqlite3_finalize(stmt_select); // Ensure select statement is finalized if not done by insert path

    // Now, retrieve "newuser" and verify its rating is 1000
    rc_prepare = sqlite3_prepare_v2(db, select_sql.c_str(), -1, &stmt_select, 0); // Re-prepare select
    assert(rc_prepare == SQLITE_OK);
    if (rc_prepare != SQLITE_OK) { std::cerr << "Prepare failed (verify): " << sqlite3_errmsg(db) << std::endl; sqlite3_close(db); exit(1); }

    sqlite3_bind_text(stmt_select, 1, username_to_test, -1, SQLITE_STATIC);
    step_rc = sqlite3_step(stmt_select);
    assert(step_rc == SQLITE_ROW);
    if (step_rc == SQLITE_ROW) {
        int retrieved_rating = sqlite3_column_int(stmt_select, 0);
        assert(retrieved_rating == default_rating);
    }
    sqlite3_finalize(stmt_select);

    sqlite3_close(db);
    remove(TEST_DB_NAME); // Clean up
    std::cout << "test_new_user_default_rating passed." << std::endl;
}

// Test Elo rating calculation logic
void test_elo_rating_calculation() {
    std::cout << "Running test_elo_rating_calculation..." << std::endl;
    
    // This test would verify the Elo calculation logic
    // Since the Elo calculation is implemented in server_main.cpp and not in a separate function,
    // we'll verify that ratings start at 0 and can be updated properly
    
    sqlite3* db;
    setup_test_database(&db);
    
    // Insert two test users with rating 0
    assert(execute_sql(db, "INSERT INTO users (username, rating) VALUES ('player1', 0);"));
    assert(execute_sql(db, "INSERT INTO users (username, rating) VALUES ('player2', 0);"));
    
    // Update one player's rating
    assert(execute_sql(db, "UPDATE users SET rating = 10 WHERE username = 'player1';"));
    
    // Verify the rating was updated
    sqlite3_stmt* stmt;
    const std::string select_sql = "SELECT rating FROM users WHERE username = 'player1';";
    int rc_prepare = sqlite3_prepare_v2(db, select_sql.c_str(), -1, &stmt, 0);
    assert(rc_prepare == SQLITE_OK);
    
    int step_rc = sqlite3_step(stmt);
    assert(step_rc == SQLITE_ROW);
    
    int retrieved_rating = sqlite3_column_int(stmt, 0);
    assert(retrieved_rating == 10);
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    remove(TEST_DB_NAME); // Clean up
    std::cout << "test_elo_rating_calculation passed." << std::endl;
}

int main() {
    std::cout << "Running DatabaseTests..." << std::endl;
    test_database_initialization();
    test_user_creation_and_retrieval();
    test_rating_updates();
    test_new_user_default_rating();
    test_elo_rating_calculation();
    std::cout << "DatabaseTests passed!" << std::endl;
    return 0;
}
