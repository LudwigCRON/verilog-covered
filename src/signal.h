#ifndef __SIGNAL_H__
#define __SIGNAL_H__

/*!
 \file     signal.h
 \author   Trevor Williams  (trevorw@charter.net)
 \date     12/1/2001
 \brief    Contains functions for handling signals.
*/

#include <stdio.h>

#include "defines.h"


//! Initializes specified signal with specified values.
void signal_init( signal* sig, char* name, vector* value );

//! Creates a new signal based on the information passed to this function.
signal* signal_create( char* name, int width, int lsb, int is_static );

//! Merges two signals, placing result into base signal.
void signal_merge( signal* base, signal* in );

//! Outputs this signal information to specified file.
void signal_db_write( signal* sig, FILE* file, char* modname );

//! Reads signal information from specified file.
bool signal_db_read( char** line, module* curr_mod );

//! Adds an expression to the signal list.
void signal_add_expression( signal* sig, expression* expr );

//! Displays signal contents to standard output.
void signal_display( signal* sig );

//! Deallocates the memory used for this signal.
void signal_dealloc( signal* sig );

/* $Log$
/* Revision 1.3  2002/07/17 06:27:18  phase1geo
/* Added start for fixes to bit select code starting with single bit selection.
/* Full regression passes with addition of sbit_sel1 diagnostic.
/* */

#endif

