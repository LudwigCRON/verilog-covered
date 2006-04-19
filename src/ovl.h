#ifndef __OVL_H__
#define __OVL_H__

/*
 Copyright (c) 2006 Trevor Williams

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
 \file     ovl.h
 \author   Trevor Williams  (trevorw@charter.net)
 \date     04/13/2006
 \brief    Contains functions for handling OVL assertion extraction.
*/


#include "defines.h"


/*! \brief Returns TRUE if specified name refers to an OVL assertion module. */
bool ovl_is_assertion_name( char* name );

/*! \brief Returns TRUE if specified functional unit is an OVL assertion module. */
bool ovl_is_assertion_module( func_unit* funit );

/*! \brief Adds all assertion modules to no score list */
void ovl_add_assertions_to_no_score_list( bool rm_tasks );

/*! \brief Gathers the OVL assertion coverage summary statistics for the given functional unit. */
void ovl_get_funit_stats( func_unit* funit, float* total, int* hit );


/*
 $Log$
 Revision 1.2  2006/04/19 22:21:33  phase1geo
 More updates to properly support assertion coverage.  Removing assertion modules
 from line, toggle, combinational logic, FSM and race condition output so that there
 won't be any overlap of information here.

 Revision 1.1  2006/04/18 21:59:54  phase1geo
 Adding support for environment variable substitution in configuration files passed
 to the score command.  Adding ovl.c/ovl.h files.  Working on support for assertion
 coverage in report command.  Still have a bit to go here yet.

*/

#endif

