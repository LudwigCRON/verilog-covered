#ifndef __COV_DB_H__
#define __COV_DB_H__

/*
 Copyright (c) 2006-2009 Trevor Williams

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by the Free Software
 Foundation; either version 2 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program;
 if not, write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*!
 \file     cov_db.h
 \author   Trevor Williams  (phase1geo@gmail.com)
 \date     8/5/2009
 \brief    Contains functions for handling the coverage database files.
*/

#include <stdio.h>

#include "defines.h"


extern cov_db**     cov_db_list;
extern unsigned int curr_cov_db;


/*!
 List of VCD tokens.
*/
enum cov_db_tokens {
  T_EOF,            /*!< 0 : End-of-file */
  T_STRING,         /*!< 1 : Other string */
  T_UNKNOWN         /*!< 2 : Unknown */
};


/*! \brief Allocates and initializes a coverage database. */
cov_db* cov_db_create();

/*! \brief Allocates a new database and adds it to the coverage database array. */
void cov_db_add();

/*! \brief Allocates memory for the coverage database at position 0 */
void cov_db_alloc();

/*! \brief Returns the index of the specified ul entry in the ul array. */
ulong cov_db_get_ul_index(
  ulong* addr
);

/*! \brief Returns the index in the ul array that the given vector should use. */
ulong cov_db_get_score_ul_index(
  vector* vec
);

/*! \brief Resizes the given vector (occurs when an expression is resized to match signal width */
void cov_db_resize_vector( 
  const vector* vec,
  unsigned int  new_width
);

/*! \brief Adds coverage data to the coverage database for the specified FSM */
void cov_db_add_fsm(
  const fsm* table
);
 
/*! \brief Adds coverage data to the coverage database for the specified expression */
void cov_db_add_expr(
  const expression* expr
);
  
/*! \brief Adds coverage data to the coverage database for the specified signal */
void cov_db_add_sig(
  const vsignal* sig
);

/*! \brief Writes the contents of the given coverage database. */
void cov_db_write(
  const char* odb
);

/*!
 \brief Reads the contents of the given coverage database, allocates and stores
        the read information into the given coverage database structure.
*/
void cov_db_read(
  const char* fname
);

/*!
  \brief Performs a data-for-data bit merge for all of the loaded database files, storing them in
         the first database location.
*/
void cov_db_merge();

/*! \brief Deallocates the given coverage database. */
void cov_db_dealloc(
  cov_db* cdb
);

/*! \brief Closes out all coverage databases. */
void cov_db_close();

#endif

