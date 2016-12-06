#include "sqlite3.h"

#include <stdio.h>

int execute_statement(sqlite3 *db, const char* zSql) {
  int nRow, nCol; /* the number of rows and columns in the result table */
  char **zResult; /* the query results */
  char *zErrorMsg = 0; /* the error message */

  /* print the statement */
  printf("> %s\n", zSql);

  /* get the table */
  int res = sqlite3_get_table(db, zSql, &zResult, &nRow, &nCol, &zErrorMsg);
  if( res == SQLITE_OK ) {
    /* print the table */
    printf("Rows: %d, Cols: %d\n", nRow, nCol);
    printf("ErrorMsg: %s\n", zErrorMsg);
    printf("zResult: %p\n", zResult);
    printf("\n");

    nCol++;
    int i;
    for( i = 0; i < (nRow * nCol); i++ ) {
      printf("|| %s ||", zResult[i]);
      if( i % nCol == 0 ) {
        printf("\n");
      }
    }
  }

  if( zResult ) {
    sqlite3_free_table(zResult);
  }

  if( zErrorMsg ) {
    sqlite3_free(zErrorMsg);
  }

  printf("\n");
  return res;
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