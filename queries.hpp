#ifndef QUERIES_HPP
#define QUERIES_HPP

#include <cstdio>
#include <string>

#include "db.hpp"

// execute_* //////////////////////////////////////////////////////////////////

void execute_select(int fout, database_t* const db, const char* const field,
                    const char* const value);

void execute_update(int fout, database_t* const db, const char* const ffield,
                    const char* const fvalue, const char* const efield, const char* const evalue);

void execute_insert(int fout, database_t* const db, const char* const fname,
                    const char* const lname, const unsigned id, const char* const section,
                    const tm birthdate);

void execute_delete(int fout, database_t* const db, const char* const field,
                    const char* const value);

// parse_and_execute_* ////////////////////////////////////////////////////////

void parse_and_execute_select(int fout, database_t* db, const char* const query);

void parse_and_execute_update(int fout, database_t* db, const char* const query);

void parse_and_execute_insert(int fout, database_t* db, const char* const query);

void parse_and_execute_delete(int fout, database_t* db, const char* const query);

void parse_and_execute(int fout, database_t* db, const char* const query);

// query_fail_* ///////////////////////////////////////////////////////////////

/** Those methods write a descriptive error message on fout */

void query_fail_bad_query_type(int const fout);

void query_fail_bad_format(int const fout, const char* const query_type);

void query_fail_too_long(int const fout, const char* const query_type);

void query_fail_bad_filter(int const fout, const char* const field, const char* const filter);

void query_fail_bad_update(int const fout, const char* const field, const char* const filter);

#endif
