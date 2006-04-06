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
 \file    vpi.c
 \author  Trevor Williams  (trevorw@charter.net)
 \date    5/6/2005
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "vpi_user.h"
#ifdef CVER
#include "cv_vpi_user.h"
#endif

#include "defines.h"
#include "link.h"
#include "util.h"
#include "instance.h"
#include "symtable.h"

char        in_db_name[1024];
char        out_db_name[1024];
int         last_time            = -1;
funit_inst* vpi_curr_funit       = NULL;

/* These are needed for compile purposes only */
char*       merge_in0            = NULL;
char*       merge_in1            = NULL;
bool        report_gui           = FALSE;
funit_inst* instance_root        = NULL;
funit_link* funit_head           = NULL;
funit_link* funit_tail           = NULL;
str_link*   no_score_head        = NULL;
str_link*   no_score_tail        = NULL;
bool        one_instance_found   = FALSE;
int         timestep_update      = 0;
bool        report_covered       = FALSE;
bool        flag_use_line_width  = FALSE;
int         line_width           = DEFAULT_LINE_WIDTH;
bool        report_instance      = FALSE;
int         flag_race_check      = WARNING;
tnode*      def_table            = NULL;
int         fork_depth           = -1;
int*        fork_block_depth;
int         block_depth          = 0;
bool        flag_exclude_initial = FALSE;
bool        flag_exclude_always  = FALSE;
bool        flag_exclude_assign  = FALSE;

extern bool         flag_scored;
extern bool         debug_mode;
extern symtable*    vcd_symtab;
extern int          vcd_symtab_size;
extern symtable**   timestep_tab;
extern char*        curr_inst_scope;
extern funit_inst*  curr_instance;
extern char*        user_msg[USER_MSG_LENGTH];


PLI_INT32 covered_value_change( p_cb_data cb ) {

#ifdef VPI_DEBUG
  vpi_printf( "In covered_value_change, name: %s, time: %d, value: %s\n",
              vpi_get_str( vpiFullName, cb->obj ), cb->time->low, cb->value->value.str );
#endif

  if( cb->time->low != last_time ) {
    if( last_time >= 0 ) {
      db_do_timestep( last_time );
    }
    last_time = cb->time->low;
  }
  
  /* Set symbol value */
  db_set_symbol_string( cb->user_data, cb->value->value.str );

  return( 0 );

}

PLI_INT32 covered_end_of_sim( p_cb_data cb ) {

#ifdef VPI_DEBUG
  vpi_printf( "At end of simulation, writing CDD contents\n" );
#endif

  /* Flush any pending statement trees that are waiting for delay */
  db_do_timestep( -1 );

  /* Indicate that this CDD contains scored information */
  flag_scored = TRUE;

  /* Write contents to database file */
  if( !db_write( out_db_name, FALSE ) ) {
    vpi_printf( "Unable to write database file" );
    exit( 1 );
  }

  /* Deallocate memory */
  symtable_dealloc( vcd_symtab );
  if( timestep_tab != NULL ) {
    free_safe( timestep_tab );
  }

  return( 0 );

}

PLI_INT32 covered_cb_error_handler( p_cb_data cb ) {

  struct t_vpi_error_info einfotab;
  struct t_vpi_error_info *einfop;
  char   s1[128];

  einfop = &einfotab;
  vpi_chk_error( einfop );

  if( einfop->state == vpiCompile ) {
    strcpy( s1, "vpiCompile" );
  } else if( einfop->state == vpiPLI ) {
    strcpy( s1, "vpiPLI" );
  } else if( einfop->state == vpiRun ) {
    strcpy( s1, "vpiRun" );
  } else {
    strcpy( s1, "**unknown**" );
  }

  vpi_printf( "**ERR(%s) %s (level %d) at **%s(%d):\n  %s\n",
    einfop->code, s1, einfop->level, einfop->file, einfop->line, einfop->message );

  /* If serious error give up */
  if( (einfop->level == vpiError) || (einfop->level == vpiSystem) || (einfop->level == vpiInternal) ) {
    vpi_printf( "**FATAL: encountered error - giving up\n" );
    vpi_control( vpiFinish, 0 );
  }

  return( 0 );

}

char* gen_next_symbol() {

  static char symbol[21]   = {32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,'\0'};
  static int  symbol_index = 19;
  int         i            = 19;

  while( (i >= symbol_index) && (symbol[i] == 126) ) {
    symbol[i] = 33;
    if( (i - 1) < symbol_index ) {
      symbol_index--;
      if( symbol_index < 0 ) {
        return( NULL );
      }
    }
    i--;
  }
  symbol[i]++;

  return( strdup_safe( (symbol + symbol_index), __FILE__, __LINE__ ) );

}

void covered_create_value_change_cb( vpiHandle sig ) {

  p_cb_data cb;
  vsignal   vsig;
  sig_link* vsigl;
  char*     symbol;

  /* Find current signal in coverage database */
  vsig.name = vpi_get_str( vpiName, sig );
  if( (vsigl = sig_link_find( &vsig, vpi_curr_funit->funit->sig_head )) != NULL ) {

#ifdef VPI_DEBUG
    vpi_printf( "In covered_create_value_change_cb, adding callback for signal: %s\n", vsigl->sig->name );
#endif

    /* Generate new symbol */
    if( (symbol = gen_next_symbol()) == NULL ) {
      vpi_printf( "INTERNAL ERROR:  Unable to generate unique symbol name\n" );
      /* Need to increase number of characters in symbol array */
      vpi_control( vpiFinish, 0 );
    }

    /* Add signal/symbol to symtab database */
    db_assign_symbol( vsigl->sig->name, symbol, ((vsigl->sig->value->width + vsigl->sig->lsb) - 1), vsigl->sig->lsb ); 

    /* Add a callback for a value change to this net */
    cb                   = (p_cb_data)malloc( sizeof( s_cb_data ) );
    cb->reason           = cbValueChange;
    cb->cb_rtn           = covered_value_change;
    cb->obj              = sig;
    cb->time             = (p_vpi_time)malloc( sizeof( s_vpi_time ) ); 
    cb->time->type       = vpiSimTime;
    cb->time->high       = 0;
    cb->time->low        = 0;
    cb->value            = (p_vpi_value)malloc( sizeof( s_vpi_value ) );
    cb->value->format    = vpiBinStrVal;
    cb->value->value.str = NULL;
    cb->user_data        = symbol;
    vpi_register_cb( cb );

  }

}

void covered_parse_task_func( vpiHandle mod ) {

  vpiHandle   iter, handle;
  vpiHandle   liter, scope;
  funit_inst* tmp_funit;
	
  /* Parse all internal scopes for tasks and functions */
  if( (iter = vpi_iterate( vpiInternalScope, mod )) != NULL ) {

    while( (scope = vpi_scan( iter )) != NULL ) {
      
#ifdef VPI_DEBUG
      vpi_printf( "Parsing task/function %s\n", vpi_get_str( vpiFullName, scope ) );
#endif

      tmp_funit = vpi_curr_funit;

      if( (vpi_curr_funit = instance_find_scope( instance_root, vpi_get_str( vpiFullName, scope ) )) != NULL ) {

        /* Set current scope in database */
        if( curr_inst_scope != NULL ) {
          free_safe( curr_inst_scope );
        }
        curr_inst_scope = strdup_safe( vpi_get_str( vpiFullName, scope ), __FILE__, __LINE__ );

        /* Parse signals */
        if( (liter = vpi_iterate( vpiReg, scope )) != NULL ) {
          while( (handle = vpi_scan( liter )) != NULL ) {
#ifdef VPI_DEBUG
            vpi_printf( "  Found reg %s, file: %s, line: %d\n",
              vpi_get_str( vpiFullName, handle ), vpi_get_str( vpiFile, handle ), vpi_get( vpiLineNo, handle ) );
#endif
            covered_create_value_change_cb( handle );
          }
        }

        /* Recursively check scope */
        if( (liter = vpi_iterate( vpiInternalScope, scope )) != NULL ) {
          while( (handle = vpi_scan( liter )) != NULL ) {
            covered_parse_task_func( handle );
          }
        }

      }

      vpi_curr_funit = tmp_funit;
 
    }

  }

}

void covered_parse_signals( vpiHandle mod ) {

  vpiHandle iter, handle;

  /* Parse nets */
  if( (iter = vpi_iterate( vpiNet, mod )) != NULL ) {

    while( (handle = vpi_scan( iter )) != NULL ) {

#ifdef VPI_DEBUG
      vpi_printf( "  Found net: %s\n", vpi_get_str( vpiName, handle ) );
#endif

      covered_create_value_change_cb( handle );

    }

  }

  /* Parse regs */
  if( (iter = vpi_iterate( vpiReg, mod )) != NULL ) {

    while( (handle = vpi_scan( iter )) != NULL ) {

#ifdef VPI_DEBUG
      vpi_printf( "  Found reg: %s\n", vpi_get_str( vpiName, handle ) );
#endif

      covered_create_value_change_cb( handle );

    }

  }

}

void covered_parse_instance( vpiHandle inst ) {

  vpiHandle  iter, handle;
  PLI_BYTE8* funit_path = vpi_get_str( vpiFullName, inst );

  /* Find current module in stored design */
  if( (vpi_curr_funit = instance_find_scope( instance_root, (char*)funit_path )) != NULL ) {

#ifdef VPI_DEBUG
    vpi_printf( "Found module to be covered: %s, full name: %s\n", vpi_get_str( vpiName, inst ), funit_path );
#endif

    /* Set database curr_instance */
    curr_instance      = vpi_curr_funit;
    one_instance_found = TRUE;

    /* Set current scope in database */
    if( curr_inst_scope != NULL ) {
      free_safe( curr_inst_scope );
    }
    curr_inst_scope = strdup_safe( vpi_get_str( vpiFullName, inst ), __FILE__, __LINE__ );

    /* Parse all signals */
    covered_parse_signals( inst );

    /* Parse all functions/tasks */
    covered_parse_task_func( inst );

  }

  if( (iter = vpi_iterate( vpiModule, inst )) != NULL ) {

    while( (handle = vpi_scan( iter )) != NULL ) {
      covered_parse_instance( handle );
    }

  }

}

PLI_INT32 covered_sim_calltf( PLI_BYTE8* name ) {

  vpiHandle       systf_handle, arg_iterator, module_handle;
  vpiHandle       arg_handle;
  s_vpi_vlog_info info;
  p_cb_data       cb;
  int             i;
  char*           argvptr;

#ifdef VPI_DEBUG
  vpi_printf( "In covered_sim_calltf, name: %s\n", name );
  debug_mode = TRUE;
#endif

  systf_handle = vpi_handle( vpiSysTfCall, NULL );
  arg_iterator = vpi_iterate( vpiArgument, systf_handle );

  /* Create callback that will handle the end of simulation */
  cb            = (p_cb_data)malloc( sizeof( s_cb_data ) );
  cb->reason    = cbEndOfSimulation;
  cb->cb_rtn    = covered_end_of_sim;
  cb->obj       = NULL;
  cb->time      = NULL;
  cb->value     = NULL;
  cb->user_data = NULL;
  vpi_register_cb( cb );

#ifdef TBD
  /* Create error handling callback */
  cb            = (p_cb_data)malloc( sizeof( s_cb_data ) );
  cb->reason    = cbError;
  cb->cb_rtn    = covered_cb_error_handler;
  cb->obj       = NULL;
  cb->time      = NULL;
  cb->value     = NULL;
  cb->user_data = NULL;
  vpi_register_cb( cb );
#endif

  /* Get name of CDD database file from system call arguments */
  if( (arg_handle = vpi_scan( arg_iterator )) != NULL ) {
    s_vpi_value data;
    data.format = vpiStringVal;
    vpi_get_value( arg_handle, &data );
    strcpy( in_db_name, data.value.str );
  }

  /* Get name of CDD database to write to (default is cov.cdd) */
  strcpy( out_db_name, "cov.cdd" );
  if( vpi_get_vlog_info( &info ) ) {
    for( i=1; i<info.argc; i++ ) {
      argvptr = info.argv[i];
      if( strncmp( "+covered_outdb=", argvptr, 15 ) == 0 ) {
        argvptr += 15;
        strcpy( out_db_name, argvptr );
      }
    }
  }

#ifdef VPI_DEBUG
  vpi_printf( "========  Reading in database %s  ========\n", in_db_name );
#endif

  /* Initialize all global information variables */
  info_initialize();
    
  /* Read in contents of specified database file */
  if( !db_read( in_db_name, READ_MODE_MERGE_NO_MERGE ) ) {
    vpi_printf( "Unable to read database file" );
    exit( 1 );
  }   

  /* Bind expressions to signals/functional units */
  bind( TRUE );

  /* Add static values to simulator */
  sim_add_statics();

  /* Create initial symbol table */
  vcd_symtab = symtable_create();

  /* Parse child instances - associate a signal in the design with a signal in Covered */
  while( (module_handle = vpi_scan( arg_iterator )) != NULL ) {
    covered_parse_instance( module_handle );
  }

  /* Create timestep symbol table array */
  if( vcd_symtab_size > 0 ) {
    timestep_tab = malloc_safe_nolimit( (sizeof( symtable*) * vcd_symtab_size), __FILE__, __LINE__ );
  }

#ifdef VPI_DEBUG
  vpi_printf( "\n*************************************************************\n\n" );
#endif

  return 0;

}

void covered_register() {

  s_vpi_systf_data tf_data;

  tf_data.type      = vpiSysTask;
  tf_data.tfname    = "$covered_sim";
  tf_data.calltf    = covered_sim_calltf;
  tf_data.compiletf = 0;
  tf_data.sizetf    = 0;
  tf_data.user_data = "$covered_sim";
  vpi_register_systf( &tf_data );

}

#ifndef VCS
void (*vlog_startup_routines[])() = {
	covered_register,
	0
};
#endif

#ifdef CVER
void vpi_compat_bootstrap(void)
{
 int i;

 for (i = 0;; i++)
  {
   if (vlog_startup_routines[i] == NULL) break;
   vlog_startup_routines[i]();
  }
}
#endif

