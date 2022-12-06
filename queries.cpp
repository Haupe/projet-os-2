#include "queries.hpp"

#include <string>
#include <iomanip>
#include <ctime>
#include <iostream>
#include <sstream>

//#include "io.hpp"

// execute_* ///////////////////////////////////////////////////////////////////

std::string execute_select(FILE* fout, database_t* const db, const char* const field,
                    const char* const value) {
  std::function<bool(const student_t&)> predicate = get_filter(field, value);
  if (!predicate) {
    query_fail_bad_filter(fout, field, value);
    return "Query failed bad filter";
  }
  int i = 0;
  for (const student_t& s : db->data) {
    if (predicate(s)) {
      i++;
    }
  }
  return std::to_string(i) + "student(s) selected";
}

std::string execute_update(FILE* fout, database_t* const db, const char* const ffield, const char* const fvalue, const char* const efield, const char* const evalue) {
  std::function<bool(const student_t&)> predicate = get_filter(ffield, fvalue);
  if (!predicate) {
    query_fail_bad_filter(fout, ffield, fvalue);
    return "Query failed bad filter";
  }
  std::function<void(student_t&)> updater = get_updater(efield, evalue);
  if (!updater) {
    query_fail_bad_update(fout, efield, evalue);
    return "Query failed bad updater";
  }
  int i=0;
  for (student_t& s : db->data) {
    if (predicate(s)) {
      updater(s);
      i ++;
    }
  }
  return std::to_string(i) + "student(s) updated";
}

std::string execute_insert(FILE* fout, database_t* const db, const char* const fname,
                    const char* const lname, const unsigned id, const char* const section,
                    const tm birthdate) {
  db->data.emplace_back();
  student_t *s = &db->data.back();
  s->id = id;
  snprintf(s->fname, sizeof(s->fname), "%s", fname);
  snprintf(s->lname, sizeof(s->lname), "%s", lname);
  snprintf(s->section, sizeof(s->section), "%s", section);
  s->birthdate = birthdate;
  
  std::ostringstream oss ;
  oss << std::put_time(&s->birthdate, "%d-%m-%Y");
  return s->fname + (std::string)s->lname + std::to_string(s->id) + s->section  + oss.str() ;
}

std::string execute_delete(FILE* fout, database_t* const db, const char* const field,
                    const char* const value) {
  std::function<bool(const student_t&)> predicate = get_filter(field, value);
  if (!predicate) {
    query_fail_bad_filter(fout, field, value);
    return "Query failed bad filter";
  }

  std::string result = execute_select(fout, db, field, value); // temporaire

  auto new_end = remove_if(db->data.begin(), db->data.end(), predicate);
  db->data.erase(new_end, db->data.end());
  
  return result;
}

// parse_and_execute_* ////////////////////////////////////////////////////////

std::string parse_and_execute_select(FILE* fout, database_t* db, const char* const query) {
  char ffield[32], fvalue[64];  // filter data
  int  counter;
  if (sscanf(query, "select %31[^=]=%63s%n", ffield, fvalue, &counter) != 2) {
    query_fail_bad_format(fout, "select");
    return "Query failed : Bad format";
  } else if (static_cast<unsigned>(counter) < strlen(query)) {
    query_fail_too_long(fout, "select");
    return "Query failed : Query too long";
  } else {
    return execute_select(fout, db, ffield, fvalue);
  }
}

std::string parse_and_execute_update(FILE* fout, database_t* db, const char* const query) {
  char ffield[32], fvalue[64];  // filter data
  char efield[32], evalue[64];  // edit data
  int counter;
  if (sscanf(query, "update %31[^=]=%63s set %31[^=]=%63s%n", ffield, fvalue, efield, evalue,
             &counter) != 4) {
    query_fail_bad_format(fout, "update");
    return "Query failed : Bad format";
  } else if (static_cast<unsigned>(counter) < strlen(query)) {
    query_fail_too_long(fout, "update");
    return "Query failed : Query too long";
  } else {
    return execute_update(fout, db, ffield, fvalue, efield, evalue);
  }
}

std::string parse_and_execute_insert(FILE* fout, database_t* db, const char* const query) {
  char      fname[64], lname[64], section[64], date[11];
  unsigned  id;
  tm        birthdate;
  int       counter;
  if (sscanf(query, "insert %63s %63s %u %63s %10s%n", fname, lname, &id, section, date, &counter) != 5 || strptime(date, "%d/%m/%Y", &birthdate) == NULL) {
    query_fail_bad_format(fout, "insert");
    return "Query failed bad format";
  } else if (static_cast<unsigned>(counter) < strlen(query)) {
    query_fail_too_long(fout, "insert");
    return "Query failed, query too long" + std::to_string(counter) + std::to_string(strlen(query));
  } else {
    return execute_insert(fout, db, fname, lname, id, section, birthdate);
  }
}

std::string parse_and_execute_delete(FILE* fout, database_t* db, const char* const query) {
  char ffield[32], fvalue[64]; // filter data
  int counter;
  if (sscanf(query, "delete %31[^=]=%63s%n", ffield, fvalue, &counter) != 2) {
    query_fail_bad_format(fout, "delete");
    return "Query failed : Bad format";
  } else if (static_cast<unsigned>(counter) < strlen(query)) {
    query_fail_too_long(fout, "delete");
    return "Query failed : Query too long";
  } else {
    return execute_delete(fout, db, ffield, fvalue);
  }
}

std::string parse_and_execute(FILE* fout, database_t* db, const char* const query) {
  if (strncmp("select", query, sizeof("select")-1) == 0) {
    return parse_and_execute_select(fout, db, query);
  } else if (strncmp("update", query, sizeof("update")-1) == 0) {
    return parse_and_execute_update(fout, db, query);
  } else if (strncmp("insert", query, sizeof("insert")-1) == 0) {
    return parse_and_execute_insert(fout, db, query);
  } else if (strncmp("delete", query, sizeof("delete")-1) == 0) {
    return parse_and_execute_delete(fout, db, query);
  } else {
    query_fail_bad_query_type(fout);
    return "Query failed bad query type";
  }
}

// query_fail_* ///////////////////////////////////////////////////////////////

void query_fail_bad_query_type(FILE* const fout) {
}

void query_fail_bad_format(FILE* const fout, const char * const query_type) {
}

void query_fail_too_long(FILE* const fout, const char * const query_type) {
}

void query_fail_bad_filter(FILE* const fout, const char* const field, const char* const filter) {
}

void query_fail_bad_update(FILE* const fout, const char* const field, const char* const filter) {
}

