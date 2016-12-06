#include "sqlite3.h"

#include <stdio.h>

int run_example_statement(sqlite3 *db) {
  sqlite3_stmt *stmt;
  const char* zSql = "CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL);";
  if( sqlite3_prepare(db, zSql, -1, &stmt, 0) != SQLITE_OK ) {
    return SQLITE_ERROR;
  }

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

  if( sqlite3_finalize(stmt) != SQLITE_OK ) {
    return SQLITE_ERROR;
  }

  return SQLITE_OK;
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