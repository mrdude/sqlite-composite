#include "sqlite3.h"

#include <stdio.h>

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
  while( !done ) {
    switch( sqlite3_step(stmt) ) {
      case SQLITE_BUSY:
        break;
      case SQLITE_ROW:
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

int run_example_statement(sqlite3 *db) {
  return execute_statement("CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL);");
}

int main(void) {
  sqlite3* db;
  sqlite3_open("dh", &db);
  printf("Hello whirled!\n");

  printf("%s\n", run_example_statement(db) == SQLITE_OK ? "ok" : "error");

  printf("Goodbye!\n");
  sqlite3_close(db);
  return 0;
}