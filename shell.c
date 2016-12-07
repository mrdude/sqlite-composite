#include "sqlite3.h"

#include <stdio.h>

void print_column_names(sqlite3_stmt *stmt) {
  const int col_count = sqlite3_column_count(stmt);
  int i;

  for( i = 0; i < col_count; i++ ) {
    const char* str = sqlite3_column_name(stmt, i);
    if( i > 0 ) printf("| ");
    printf("%-15s", str);
    if( i < col_count ) printf(" |");
  }
}

void print_statement_columns(sqlite3_stmt *stmt) {
  const int col_count = sqlite3_column_count(stmt);
  int i;

  for( i = 0; i < col_count; i++ ) {
    const char* str = sqlite3_column_text(stmt, i);
    if( i > 0 ) printf("| ");
    printf("%-15s", str);
    if( i < col_count ) printf(" |");
  }
}

int execute_statement(sqlite3 *db, const char* zSql) {
  sqlite3_stmt *stmt;

  /* print the statement */
  printf("> %s\n", zSql);
  
  /* prepare the statement */
  if( sqlite3_prepare(db, zSql, -1, &stmt, 0) != SQLITE_OK ) {
    return SQLITE_ERROR;
  }

  /* execute the statement */
  int done = 0;
  int row_count = 0;
  while( !done ) {
    switch( sqlite3_step(stmt) ) {
      case SQLITE_BUSY:
        break;
      case SQLITE_ROW:
        if( row_count == 0 ) {
          printf("\t");
          print_column_names(stmt);
          printf("\n\t==\n");
        }

        printf("\t");
        print_statement_columns(stmt);
        printf("\n");
        row_count++;
        break;
      case SQLITE_DONE:
        done = 1;
        break;
      case SQLITE_ERROR:
      case SQLITE_MISUSE:
        return SQLITE_ERROR;
    }
  }

  /* free the statement from memory */
  if( sqlite3_finalize(stmt) != SQLITE_OK ) {
    return SQLITE_ERROR;
  }

  return SQLITE_OK;
}

int run_example_statements(sqlite3 *db) {
  const char* zStatements[] = {
    "CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL);",
    "INSERT INTO users (name) VALUES ('Billy'), ('Vegeta'), ('Jack'), ('Some other guy');",
    "SELECT * FROM users;",
    "SELECT id FROM users WHERE name == 'Billy';",
    "CREATE TABLE scores (id INTEGER PRIMARY KEY AUTOINCREMENT, user_id INTEGER, score INTEGER NOT NULL, FOREIGN KEY(user_id) REFERENCES users(id));",
    "INSERT INTO scores (user_id, score) VALUES (1, 34), (2, 9001), (2, 9002), (3, 10), (3, 5);",
    "SELECT scores.id, users.name, scores.score FROM users, scores WHERE scores.user_id == users.id;",
    "SELECT users.name, scores.score FROM scores JOIN users ON users.id == scores.user_id ORDER BY scores.score DESC LIMIT 3;"
  };

  const int statement_count = 8;

  int i;
  for( i = 0; i < statement_count; i++ ) {
    if( execute_statement(db, zStatements[i]) != SQLITE_OK ) {
      return SQLITE_ERROR;
    }
  }

  return SQLITE_OK;
}

int main(void) {
  sqlite3* db;
  sqlite3_open("dh", &db);
  printf("Hello whirled!\n");

  printf("%s\n", run_example_statements(db) == SQLITE_OK ? "All tests passed" : "A test failed");

  printf("Goodbye!\n");
  sqlite3_close(db);
  return 0;
}