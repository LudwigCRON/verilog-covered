#ifndef __ASSERTION_H__
#define __ASSERTION_H__

/*!
 \file     assertion.h
 \author   Trevor Williams  (trevorw@charter.net)
 \date     4/4/2006
 \brief    Contains functions for handling assertion coverage.
*/


#include "defines.h"


/*! \brief Parses -A command-line option to score command */
void assertion_parse( char* arg );

/*! \brief Parses an in-line attribute for assertion coverage information */
void assertion_parse_attr( attr_param* ap, func_unit* funit );

/*
 $Log$
 Revision 1.1  2006/04/05 15:19:18  phase1geo
 Adding support for FSM coverage output in the GUI.  Started adding components
 for assertion coverage to GUI and report functions though there is no functional
 support for this at this time.

*/

#endif

