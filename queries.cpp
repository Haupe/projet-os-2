#define STRING_SIZE 256

#include "queries.hpp"

#include <string>
#include <iomanip>
#include <ctime>
#include <iostream>
#include <sstream>
#include <unistd.h>

//#include "io.hpp"

// execute_* ///////////////////////////////////////////////////////////////////

void execute_select(int fout, database_t* const db, const char* const field,
                    const char* const value) {
  std::function<bool(const student_t&)> predicate = get_filter(field, value);
  if (!predicate) {
    query_fail_bad_filter(fout, field, value);
    return;
  }
  int i = 0;
  std::string result = "";
  char buffer[STRING_SIZE] = ""; 
  for (const student_t& s : db->data) {
    if (predicate(s)) {
      student_to_str(buffer, &s, STRING_SIZE);
      result = buffer;
      result += "\n";
      i++;
      write(fout, result.c_str(), result.size());
    }
  }
  char puffer[STRING_SIZE]="";
  result = "";
  snprintf(puffer, STRING_SIZE, "%i student(s) selected \n", i);
  result += puffer;
  write(fout, result.c_str(), result.size());

}

void execute_update(int fout, database_t* const db, const char* const ffield, const char* const fvalue, const char* const efield, const char* const evalue) {
  std::function<bool(const student_t&)> predicate = get_filter(ffield, fvalue);
  if (!predicate) {
    query_fail_bad_filter(fout, ffield, fvalue);
    return;
  }
  std::function<void(student_t&)> updater = get_updater(efield, evalue);
  if (!updater) {
    query_fail_bad_update(fout, efield, evalue);
    return;
  }
  int i=0;
  for (student_t& s : db->data) {
    if (predicate(s)) {
      updater(s);
      i ++;
    }
  }
  std::string result = "";
  char buffer[STRING_SIZE] = "";
  snprintf(buffer, STRING_SIZE, "%i student(s) updated \n", i);
  result += buffer;
  write(fout, result.c_str(), result.size());
}

void execute_insert(int fout, database_t* const db, const char* const fname,
                    const char* const lname, const unsigned id, const char* const section,
                    const tm birthdate) {
  for(student_t& s : db->data){
    if (s.id == id){
      std::string result = "Query failed : id already in database.\n";
      write(fout,result.c_str(), result.size());
      return;
    }
  }
  db->data.emplace_back();
  student_t *s = &db->data.back();
  s->id = id;
  snprintf(s->fname, sizeof(s->fname), "%s", fname);
  snprintf(s->lname, sizeof(s->lname), "%s", lname);
  snprintf(s->section, sizeof(s->section), "%s", section);
  s->birthdate = birthdate;
  
  std::string result = "";
  char buffer[STRING_SIZE] = "";
  student_to_str(buffer, s, STRING_SIZE);
  result += buffer;
  result += "\n";
  write(fout,result.c_str(), result.size());
}

void execute_delete(int fout, database_t* const db, const char* const field,
                    const char* const value) {
  std::function<bool(const student_t&)> predicate = get_filter(field, value);
  if (!predicate) {
    query_fail_bad_filter(fout, field, value);
    return;
  }

  char buffer[STRING_SIZE] = ""; 
  std::string result = "";
  int i=0;
  for (const student_t& s : db->data) {
    if (predicate(s)) {
      student_to_str(buffer, &s, STRING_SIZE);
      result = buffer;
      result += "\n";
      i ++;
      write(fout, result.c_str(), result.size());
    }
  }
  char puffer[STRING_SIZE] = "";
  snprintf(puffer, STRING_SIZE, "%i student(s) deleted \n", i);
  result = puffer;
  write(fout, result.c_str(), result.size());
  auto new_end = remove_if(db->data.begin(), db->data.end(), predicate);
  db->data.erase(new_end, db->data.end());
  
}

// parse_and_execute_* ////////////////////////////////////////////////////////

void parse_and_execute_select(int fout, database_t* db, const char* const query) {
  char ffield[32], fvalue[64];  // filter data
  int  counter;
  if (sscanf(query, "select %31[^=]=%63s%n", ffield, fvalue, &counter) != 2) {
    query_fail_bad_format(fout, "select");
  } else if (static_cast<unsigned>(counter) < strlen(query)) {
    query_fail_too_long(fout, "select");
  } else {
    execute_select(fout, db, ffield, fvalue);
  }
}

void parse_and_execute_update(int fout, database_t* db, const char* const query) {
  char ffield[32], fvalue[64];  // filter data
  char efield[32], evalue[64];  // edit data
  int counter;
  if (sscanf(query, "update %31[^=]=%63s set %31[^=]=%63s%n", ffield, fvalue, efield, evalue,
             &counter) != 4) {
    query_fail_bad_format(fout, "update");
  } else if (static_cast<unsigned>(counter) < strlen(query)) {
    query_fail_too_long(fout, "update");
  } else {
    execute_update(fout, db, ffield, fvalue, efield, evalue);
  }
}

void parse_and_execute_insert(int fout, database_t* db, const char* const query) {
  char      fname[64], lname[64], section[64], date[11];
  unsigned  id;
  tm        birthdate;
  int       counter;
  if (sscanf(query, "insert %63s %63s %u %63s %10s%n", fname, lname, &id, section, date, &counter) != 5 || strptime(date, "%d/%m/%Y", &birthdate) == NULL) {
    query_fail_bad_format(fout, "insert");
  } else if (static_cast<unsigned>(counter) < strlen(query)) {
    query_fail_too_long(fout, "insert");
  } else {
    execute_insert(fout, db, fname, lname, id, section, birthdate);
  }
}

void parse_and_execute_delete(int fout, database_t* db, const char* const query) {
  char ffield[32], fvalue[64]; // filter data
  int counter;
  if (sscanf(query, "delete %31[^=]=%63s%n", ffield, fvalue, &counter) != 2) {
    query_fail_bad_format(fout, "delete");
  } else if (static_cast<unsigned>(counter) < strlen(query)) {
    query_fail_too_long(fout, "delete");
  } else {
    execute_delete(fout, db, ffield, fvalue);
  }
}

void parse_and_execute(int fout, database_t* db, const char* const query) {
  if (strncmp("select", query, sizeof("select")-1) == 0) {
    parse_and_execute_select(fout, db, query);
  } else if (strncmp("update", query, sizeof("update")-1) == 0) {
    parse_and_execute_update(fout, db, query);
  } else if (strncmp("insert", query, sizeof("insert")-1) == 0) {
    parse_and_execute_insert(fout, db, query);
  } else if (strncmp("delete", query, sizeof("delete")-1) == 0) {
    parse_and_execute_delete(fout, db, query);
  } else {
    query_fail_bad_query_type(fout);
  }
}

// query_fail_* ///////////////////////////////////////////////////////////////

void query_fail_bad_query_type(int const fout) {
  printf("Query failed bad query type\n");
  std::string error = "Query failed bad query type \n";
  write(fout, error.c_str(), error.size());
}

void query_fail_bad_format(int const fout, const char * const query_type) {
  char buffer[STRING_SIZE] = "";
  std::string error = "";
  snprintf(buffer, STRING_SIZE, "Query %s failed: bad format \n", query_type);
  printf("%s \n", buffer);
  error += buffer;
  write(fout, error.c_str(), error.size());
}

void query_fail_too_long(int const fout, const char * const query_type) {
  char buffer[STRING_SIZE] = "";
  std::string error = "";
  snprintf(buffer, STRING_SIZE, "Query %s failed: query too long \n", query_type);
  printf("%s \n", buffer);
  error += buffer;
  write(fout, error.c_str(), error.size());
}

void query_fail_bad_filter(int const fout, const char* const field, const char* const filter) {
  char buffer[STRING_SIZE] = "";
  std::string error = "";
  snprintf(buffer, STRING_SIZE, "Query failed bad filter : field = %s, filter = %s \n", field, filter);
  printf("%s \n", buffer);
  error += buffer;
  write(fout, error.c_str(), error.size());
}

void query_fail_bad_update(int const fout, const char* const field, const char* const filter) {
  char buffer[STRING_SIZE] = "";
  std::string error = "";
  snprintf(buffer, STRING_SIZE, "Query failed bad update : field = %s, filter = %s \n", field, filter);
  printf("%s \n", buffer);
  error += buffer;
  write(fout, error.c_str(), error.size());
}

