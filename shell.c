#include "sqlite3.h"

#include <stdio.h>

void print_statement_columns(sqlite3_stmt *stmt) {
  const int col_count = sqlite3_column_count(stmt);
  int i;

  for( i = 0; i < col_count; i++ ) {
    /* print column ID */
    printf("Column %d: ", i);

    /* print column type */
    switch( sqlite3_column_type(stmt, i) ) {
      case SQLITE_INTEGER: printf("Integer"); break;
      case SQLITE_FLOAT:   printf("Float  "); break;
      case SQLITE_BLOB:    printf("Blob   "); break;
      case SQLITE_NULL:    printf("Null   "); break;
      case SQLITE_TEXT:    printf("Text   "); break;
    }

    printf(" -> ");

    /* print column data */
    printf("TODO");

    printf("\n");
  }

  printf("End of Row\n-\n\n");
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
        print_statement_columns(stmt);
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

  printf("Printed %d rows\n\n", row_count);

  /* free the statement from memory */
  if( sqlite3_finalize(stmt) != SQLITE_OK ) {
    return SQLITE_ERROR;
  }

  return SQLITE_OK;
}

int run_example_statements(sqlite3 *db) {
  const char** zStatements = {
    "CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL);",
    "INSERT INTO users (name) VALUES ('Billy'), ('Vegeta'), ('Jack'), ('Some other guy');",
    "SELECT * FROM users;",
    ""
  };

  int i;
  for( i = 0; zStatements[i] != 0; i++ ) {
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