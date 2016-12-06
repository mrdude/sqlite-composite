#include "sqlite3.h"

#include <stdio.h>
#include <stdlib.h> /* for malloc() */

struct rs_row {
  int col_count;
  int* col_len;
  char** col_data;
};

static char* _copy_string(const char* str, int *str_len) {
  int len = 0;
  for( ; str[len] != 0; len++ ) {}

  (*str_len) = len;

  char* new_str = malloc( sizeof(char) * (len+1) );
  int i;
  for( i = 0; i < len; i++ ) {
    new_str[i] = str[i];
  }
  new_str[len] = 0;

  return new_str;
}

static struct rs_row* generate_row(sqlite3_stmt *stmt) {
  struct rs_row* row = malloc( sizeof(struct rs_row) );
  row->col_count = sqlite3_column_count(stmt);

  row->col_len = malloc( sizeof(int) * row->col_count );
  row->col_data = malloc( sizeof(char*) * row->col_count );

  int i;
  for( i = 0; i < row->col_count; i++ ) {
    row->col_data[i] = _copy_string( sqlite3_column_text(stmt, i), &row->col_len[i] );
  }

  return row;
}

static void free_row(struct rs_row* row) {
  int i;
  for( i = 0; i < row->col_count; i++ ) {
    free( row->col_data[i] );
  }

  free( row->col_data );
  free( row->col_len );
  free( row );
}

static void print_row(struct rs_row* row) {
  int i;
  for( i = 0; i < row->col_count; i++ ) {
    if( i != 0 ) printf("| ");
    printf("%s", row->col_data[i]);
    if( i != row->col_count-1 ) printf(" |");
  }
  printf("\n");
}

void print_column_names(sqlite3_stmt *stmt) {
  const int col_count = sqlite3_column_count(stmt);
  int i;

  for( i = 0; i < col_count; i++ ) {
    printf("| %s |", sqlite3_column_name(stmt, i));
  }

  printf("\n==\n");
}

void print_statement_columns(sqlite3_stmt *stmt) {
  const int col_count = sqlite3_column_count(stmt);
  struct rs_row* row;

  row = generate_row(stmt);
  print_row(row);
  free_row(row);
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
          print_column_names(stmt);
        }

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