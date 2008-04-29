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
 \file     vector.c
 \author   Trevor Williams  (phase1geo@gmail.com)
 \date     12/1/2001
 
 \par
 The vector is the data structure that stores all of the current value and coverage
 information for a particular signal or expression.  All simulated operations are
 performed on vector structures and as such are the most used and optimized data
 structures used by Covered.  To keep the memory image of a vector to as small as
 possible, a vector is comprised of three components:
 
 \par
   -# A least-significant bit (LSB) value which specifies the current vector's LSB.
   -# A width value which specifies the number of bits contained in the current vector.
      To get the MSB of the vector, simply add the width to the LSB and subtract one.
   -# A value array which contains the vector's current value and other coverage
      information gained during simulation.  This array is an array of 32-bit values
      (or nibbles) whose length is determined by the width of the vector divided by four.
      We divide the width by 4 because one nibble contains all of the information for
      up to 4 bits of four-state data.  For a break-down of the bits within a nibble,
      please consult the \ref nibble table.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <stdlib.h>
#include <assert.h>

#ifdef MALLOC_DEBUG
#include <mpatrol.h>
#endif

#include "defines.h"
#include "vector.h"
#include "util.h"


/*!
 Returns the number of 32-bit elements are required to store a vector with a bit width of width.
*/
#define VECTOR_SIZE32(width)      (((width & 0x1f) == 0) ? (width >> 5) : ((width >> 5) + 1))

/*!
 Returns the number of 64-bit elements are required to store a vector with a bit width of width.
*/
#define VECTOR_SIZE64(width)      (((width & 0x3f) == 0) ? (width >> 6) : ((width >> 6) + 1))


/*! Contains the structure sizes for the various vector types (vector "type" supplemental field is the index to this array */
static const unsigned int vector_type_sizes[4] = {VTYPE_INDEX_VAL_NUM, VTYPE_INDEX_SIG_NUM, VTYPE_INDEX_EXP_NUM, VTYPE_INDEX_MEM_NUM};

extern char user_msg[USER_MSG_LENGTH];


/*!
 \param vec         Pointer to vector to initialize.
 \param value       Pointer to vec_data array for vector.
 \param data        Initial value to set each bit to.
 \param owns_value  Set to TRUE if this vector is responsible for deallocating the given value array
 \param width       Bit width of specified vector.
 \param type        Type of vector to initialize this to.
 
 Initializes the specified vector with the contents of width
 and value (if value != NULL).  If value != NULL, initializes all contents 
 of value array to 0x2 (X-value).
*/
void vector_init_uint32(
  vector*   vec,
  uint32**  value,
  uint32    data_l,
  uint32    data_h,
  bool      owns_value,
  int       width,
  int       type,
  int       data_type
) { PROFILE(VECTOR_INIT);

  vec->width                = width;
  vec->suppl.all            = 0;
  vec->suppl.part.type      = type;
  vec->suppl.part.data_type = data_type;
  vec->suppl.part.owns_data = owns_value;
  vec->value.u32            = value;

  if( value != NULL ) {

    int    i, j;
    int    size  = VECTOR_SIZE32(width);
    int    num   = vector_type_sizes[type];
    uint32 lmask = 0xffffffff >> (31 - ((width - 1) & 0x1f));

    assert( width > 0 );

    for( i=0; i<(size - 1); i++ ) {
      vec->value.u32[i][VTYPE_INDEX_VAL_VALL] = data_l;
      vec->value.u32[i][VTYPE_INDEX_VAL_VALH] = data_h;
      for( j=2; j<num; j++ ) {
        vec->value.u32[i][j] = 0x0;
      }
    }

    vec->value.u32[i][VTYPE_INDEX_VAL_VALL] = data_l & lmask;
    vec->value.u32[i][VTYPE_INDEX_VAL_VALH] = data_h & lmask;
    for( j=2; j<num; j++ ) {
      vec->value.u32[i][j] = 0x0;
    }

  } else {

    assert( !owns_value );

  }

  PROFILE_END;

}

/*!
 \param width  Bit width of this vector.
 \param type   Type of vector to create (see \ref vector_types for valid values).
 \param size   Data type used to store vector value information
 \param data   If FALSE only initializes width but does not allocate a nibble array.

 \return Pointer to newly created vector.

 Creates new vector from heap memory and initializes all vector contents.
*/
vector* vector_create(
  int  width,
  int  type,
  int  data_type,
  bool data
) { PROFILE(VECTOR_CREATE);

  vector*  new_vec;       /* Pointer to newly created vector */
  uint32** value = NULL;  /* Temporarily stores newly created vector value */

  assert( width > 0 );

  new_vec = (vector*)malloc_safe( sizeof( vector ) );

  switch( data_type ) {
    case VDATA_U32 :
      {
        uint32** value = NULL;
        if( data == TRUE ) {
          int          num  = vector_type_sizes[type];
          int          size = VECTOR_SIZE32(width);
          unsigned int i;
          value = (uint32**)malloc_safe( sizeof( uint32* ) * size );
          for( i=0; i<size; i++ ) {
            value[i] = (uint32*)malloc_safe( sizeof( uint32 ) * num );
          }
        }
        vector_init_uint32( new_vec, value, 0x0, 0x0, (value != NULL), width, type, data_type );
      }
      break;
    default :  assert( 0 );
  }

  PROFILE_END;

  return( new_vec );

}

/*!
 \param from_vec  Vector to copy.
 \param to_vec    Newly created vector copy.
 
 Copies the contents of the from_vec to the to_vec.
*/
void vector_copy(
  const vector* from_vec,
  vector*       to_vec
) { PROFILE(VECTOR_COPY);

  unsigned int i, j;  /* Loop iterators */

  assert( from_vec != NULL );
  assert( to_vec != NULL );
  assert( from_vec->width == to_vec->width );
  assert( from_vec->suppl.part.type == to_vec->suppl.part.type );
  assert( from_vec->suppl.part.data_type == to_vec->suppl.part.data_type );

  switch( to_vec->suppl.part.data_type ) {
    case VDATA_U32 :
      for( i=0; i<VECTOR_SIZE32(from_vec->width); i++ ) {
        for( j=0; j<vector_type_sizes[to_vec->suppl.part.type]; j++ ) {
          to_vec->value.u32[i][j] = from_vec->value.u32[i][j];
        }
      }
      break;
    default:  assert( 0 );  break;
  }

  PROFILE_END;

}

/*!
 Copies the entire contents of a bit range from from_vec to to_vec,
 aligning the stored value starting at bit 0.
*/
void vector_copy_range(
  vector*       to_vec,    /*!< Vector to copy to */
  const vector* from_vec,  /*!< Vector to copy from */
  int           lsb        /*!< LSB of bit range to copy */
) { PROFILE(VECTOR_COPY_RANGE);

  assert( from_vec != NULL );
  assert( to_vec != NULL );
  assert( from_vec->suppl.part.type == to_vec->suppl.part.type );
  assert( from_vec->suppl.part.data_type == to_vec->suppl.part.data_type );

  switch( to_vec->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        unsigned int i, j;
        for( i=0; i<to_vec->width; i++ ) {
          unsigned int my_index     = (i >> 5);
          unsigned int their_index  = ((i + lsb) >> 5);
          unsigned int their_offset = ((i + lsb) & 0x1f);
          for( j=0; j<vector_type_sizes[to_vec->suppl.part.type]; j++ ) {
            if( (i & 0x1f) == 0 ) {
              to_vec->value.u32[my_index][j] = 0;
            }
            to_vec->value.u32[my_index][j] |= (((from_vec->value.u32[their_index][j] >> their_offset) & 0x1) << i);
          }
        }
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

}

/*!
 \param from_vec  Vector to copy.
 \param to_vec    Newly created vector copy. 
 
 Copies the contents of the from_vec to the to_vec, allocating new memory.
*/
void vector_clone(
            const vector*  from_vec,
  /*@out@*/ vector**       to_vec
) { PROFILE(VECTOR_CLONE);

  if( from_vec == NULL ) {

    /* If from_vec is NULL, just assign to_vec to NULL */
    *to_vec = NULL;

  } else {

    /* Create vector */
    *to_vec = vector_create( from_vec->width, from_vec->suppl.part.type, from_vec->suppl.part.data_type, TRUE );

    vector_copy( from_vec, *to_vec );

  }

  PROFILE_END;

}

/*!
 \param vec         Pointer to vector to display to database file.
 \param file        Pointer to coverage database file to display to.
 \param write_data  If set to TRUE, causes 4-state data bytes to be included.

 Writes the specified vector to the specified coverage database file.
*/
void vector_db_write(
  vector* vec,
  FILE*   file,
  bool    write_data
) { PROFILE(VECTOR_DB_WRITE);

  nibble mask;   /* Mask value for vector value nibbles */

  assert( vec != NULL );
  assert( vec->width > 0 );

  /* Calculate vector data mask */
  mask = write_data ? 0xff : 0xfc;
  switch( vec->suppl.part.type ) {
    case VTYPE_VAL :  mask = mask & 0x03;  break;
    case VTYPE_SIG :  mask = mask & 0x0f;  break;
    case VTYPE_EXP :  mask = mask & 0x3f;  break;
    case VTYPE_MEM :  mask = mask & 0x3f;  break;
    default        :  break;
  }

  /* Output vector information to specified file */
  /*@-formatcode@*/
  fprintf( file, "%d %hhu",
    vec->width,
    vec->suppl.all
  );
  /*@=formatcode@*/

  /* Only write our data if we own it */
  if( vec->suppl.part.owns_data == 1 ) {

    /* Output value based on data type */
    switch( vec->suppl.part.data_type ) {
      case VDATA_U32 :
        {
          uint32       dflt_h = (vec->suppl.part.is_2state == 1) ? 0x0 : 0xffffffff;
          unsigned int i, j;
          for( i=0; i<(VECTOR_SIZE32(vec->width) - 1); i++ ) {
            fprintf( file, " %x", (write_data && (vec->value.u32 != NULL)) ? vec->value.u32[i][VTYPE_INDEX_VAL_VALL] : 0 );
            fprintf( file, " %x", (write_data && (vec->value.u32 != NULL)) ? vec->value.u32[i][VTYPE_INDEX_VAL_VALH] : dflt_h );
            for( j=2; j<vector_type_sizes[vec->suppl.part.type]; j++ ) {
              if( ((mask >> j) & 0x1) == 1 ) {
                fprintf( file, " %x", (vec->value.u32 != NULL) ? vec->value.u32[i][j] : 0 );
              } else {
                fprintf( file, " 0" );
              }
            }
          }

          dflt_h >>= (31 - ((vec->width - 1) & 0x1f));

          fprintf( file, " %x", (write_data && (vec->value.u32 != NULL)) ? vec->value.u32[i][VTYPE_INDEX_VAL_VALL] : 0 );
          fprintf( file, " %x", (write_data && (vec->value.u32 != NULL)) ? vec->value.u32[i][VTYPE_INDEX_VAL_VALH] : dflt_h );
          for( j=2; j<vector_type_sizes[vec->suppl.part.type]; j++ ) {
            if( ((mask >> j) & 0x1) == 1 ) {
              fprintf( file, " %x", (vec->value.u32 != NULL) ? vec->value.u32[i][j] : 0 );
            } else {
              fprintf( file, " 0" );
            }
          }
        }
        break;
      default :  assert( 0 );  break;
    }

  }

  PROFILE_END;

}

/*!
 \param vec    Pointer to vector to create.
 \param line   Pointer to line to parse for vector information.

 \throws anonymous Throw Throw

 Creates a new vector structure, parses current file line for vector information
 and returns new vector structure to calling function.
*/
void vector_db_read(
  vector** vec,
  char**   line
) { PROFILE(VECTOR_DB_READ);

  int    width;       /* Vector bit width */
  vsuppl suppl;       /* Temporary supplemental value */
  int    chars_read;  /* Number of characters read */

  /* Read in vector information */
  if( sscanf( *line, "%d %d%n", &width, &suppl, &chars_read ) == 2 ) {

    *line = *line + chars_read;

    /* Create new vector */
    *vec              = vector_create( width, suppl.part.type, suppl.part.data_type, TRUE );
    (*vec)->suppl.all = suppl.all;

    if( suppl.part.owns_data == 1 ) {

      Try {

        switch( suppl.part.data_type ) {
          case VDATA_U32 :
            {
              unsigned int i, j;
              for( i=0; i<VECTOR_SIZE32(width); i++ ) {
                for( j=0; j<vector_type_sizes[suppl.part.type]; j++ ) {
                  if( sscanf( *line, "%x%n", &((*vec)->value.u32[i][j]), &chars_read ) == 1 ) {
                    *line += chars_read;
                  } else {
                    print_output( "Unable to parse vector information in database file.  Unable to read.", FATAL, __FILE__, __LINE__ );
                    printf( "vector Throw A\n" );
                    Throw 0;
                  }
                }
              }
            }
            break;
          default :  assert( 0 );  break;
        }

      } Catch_anonymous {
        vector_dealloc( *vec );
        *vec = NULL;
        printf( "vector Throw B\n" );
        Throw 0;
      }

    /* Otherwise, deallocate the vector data */
    } else {

      vector_dealloc_value( *vec );

    }

  } else {

    print_output( "Unable to parse vector information in database file.  Unable to read.", FATAL, __FILE__, __LINE__ );
    printf( "vector Throw C\n" );
    Throw 0;

  }

  PROFILE_END;

}

/*!
 \param base  Base vector to merge data into.
 \param line  Pointer to line to parse for vector information.
 \param same  Specifies if vector to merge needs to be exactly the same as the existing vector.

 \throws anonymous Throw Throw Throw

 Parses current file line for vector information and performs vector merge of 
 base vector and read vector information.  If the vectors are found to be different
 (width is not equal), an error message is sent to the user and the
 program is halted.  If the vectors are found to be equivalents, the merge is
 performed on the vector nibbles.
*/
void vector_db_merge(
  vector* base,
  char**  line,
  bool    same
) { PROFILE(VECTOR_DB_MERGE);

  int    width;       /* Width of read vector */
  vsuppl suppl;       /* Supplemental value of vector */
  int    chars_read;  /* Number of characters read */

  assert( base != NULL );

  if( sscanf( *line, "%d %d%n", &width, &suppl, &chars_read ) == 2 ) {

    *line = *line + chars_read;

    if( base->width != width ) {

      if( same ) {
        print_output( "Attempting to merge databases derived from different designs.  Unable to merge",
                      FATAL, __FILE__, __LINE__ );
        printf( "vector Throw D\n" );
        Throw 0;
      }

    } else if( base->suppl.part.owns_data == 1 ) {

      switch( base->suppl.part.data_type ) {
        case VDATA_U32 :
          {
            unsigned int i, j;
            uint32       value;
            for( i=0; i<VECTOR_SIZE32(width); i++ ) {
              for( j=0; j<vector_type_sizes[suppl.part.type]; j++ ) {
                if( sscanf( *line, "%x%n", &value, &chars_read ) == 1 ) {
                  *line += chars_read;
                  if( j >= 2 ) {
                    base->value.u32[i][j] |= value;
                  }
                } else {
                  print_output( "Unable to parse vector line from database file.  Unable to merge.", FATAL, __FILE__, __LINE__ );
                  printf( "vector Throw E\n" );
                  Throw 0;
                }
              }
            } 
          }
          break;
        default :  assert( 0 );  break;
      }

    }

  } else {

    print_output( "Unable to parse vector line from database file.  Unable to merge.", FATAL, __FILE__, __LINE__ );
    printf( "vector Throw F\n" );
    Throw 0;

  }

  PROFILE_END;

}

/*!
 Merges two vectors, placing the result back into the base vector.  This function is used by the GUI for calculating
 module coverage.
*/
void vector_merge(
  vector* base,  /*!< Vector which will contain the merged results */
  vector* other  /*!< Vector which will merge its results into the base vector */
) { PROFILE(VECTOR_MERGE);

  unsigned int i, j;  /* Loop iterator */

  assert( base != NULL );
  assert( base->width == other->width );

  switch( base->suppl.part.data_type ) {
    case VDATA_U32 :
      for( i=0; i<VECTOR_SIZE32(base->width); i++ ) {
        for( j=2; j<vector_type_sizes[base->suppl.part.type]; j++ ) {
          base->value.u32[i][j] |= other->value.u32[i][j];
        }
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

}

/*!
 \return Returns eval_a coverage information for specified vector and bit position.
*/
int vector_get_eval_a(
  vector* vec,   /*!< Pointer to vector to get eval_a information from */
  int     index  /*!< Index to retrieve bit from */
) { PROFILE(VECTOR_GET_EVAL_A);

  int retval;  /* Return value for this function */

  assert( vec != NULL );
  assert( vec->suppl.part.type == VTYPE_EXP );

  switch( vec->suppl.part.data_type ) {
    case VDATA_U32 :  retval = (vec->value.u32[index>>5][VTYPE_INDEX_EXP_EVAL_A] >> (index & 0x1f)) & 0x1;  break;
    default        :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \return Returns eval_b coverage information for specified vector and bit position.
*/
int vector_get_eval_b(
  vector* vec,   /*!< Pointer to vector to get eval_b information from */
  int     index  /*!< Index to retrieve bit from */
) { PROFILE(VECTOR_GET_EVAL_B);

  int retval;  /* Return value for this function */

  assert( vec != NULL );
  assert( vec->suppl.part.type == VTYPE_EXP );

  switch( vec->suppl.part.data_type ) {
    case VDATA_U32 :  retval = (vec->value.u32[index>>5][VTYPE_INDEX_EXP_EVAL_B] >> (index & 0x1f)) & 0x1;  break;
    default        :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \return Returns eval_c coverage information for specified vector and bit position.
*/
int vector_get_eval_c(
  vector* vec,   /*!< Pointer to vector to get eval_c information from */
  int     index  /*!< Index to retrieve bit from */
) { PROFILE(VECTOR_GET_EVAL_C);

  int retval;  /* Return value for this function */

  assert( vec != NULL );
  assert( vec->suppl.part.type == VTYPE_EXP );

  switch( vec->suppl.part.data_type ) {
    case VDATA_U32 :  retval = (vec->value.u32[index>>5][VTYPE_INDEX_EXP_EVAL_C] >> (index & 0x1f)) & 0x1;  break;
    default        :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \return Returns eval_a coverage information for specified vector and bit position.
*/
int vector_get_eval_d(
  vector* vec,   /*!< Pointer to vector to get eval_d information from */
  int     index  /*!< Index to retrieve bit from */
) { PROFILE(VECTOR_GET_EVAL_D);

  int retval;  /* Return value for this function */

  assert( vec != NULL );
  assert( vec->suppl.part.type == VTYPE_EXP );

  switch( vec->suppl.part.data_type ) {
    case VDATA_U32 :  retval = (vec->value.u32[index>>5][VTYPE_INDEX_EXP_EVAL_D] >> (index & 0x1f)) & 0x1;  break;
    default        :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \param vec  Pointer to vector to count eval_a/b bits in

 \return Returns the number of eval_a/b bits are set in the given vector.
*/
int vector_get_eval_ab_count(
  vector* vec
) { PROFILE(VECTOR_GET_EVAL_AB_COUNT);

  int          count = 0;  /* Number of EVAL_A/B bits set */
  unsigned int i, j;       /* Loop iterators */

  switch( vec->suppl.part.data_type ) {
    case VDATA_U32 :
      for( i=0; i<VECTOR_SIZE32( vec->width ); i++ ) {
        uint32 value_a = vec->value.u32[i][VTYPE_INDEX_EXP_EVAL_A];
        uint32 value_b = vec->value.u32[i][VTYPE_INDEX_EXP_EVAL_B];
        for( j=0; j<32; j++ ) {
          count += (value_a >> j) & 0x1;
          count += (value_b >> j) & 0x1;
        }
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( count );

}

/*!
 \param vec  Pointer to vector to count eval_a/b/c bits in

 \return Returns the number of eval_a/b/c bits are set in the given vector.
*/
int vector_get_eval_abc_count(
  vector* vec
) { PROFILE(VECTOR_GET_EVAL_ABC_COUNT);

  int          count = 0;  /* Number of EVAL_A/B/C bits set */
  unsigned int i, j;       /* Loop iterators */

  switch( vec->suppl.part.data_type ) {
    case VDATA_U32 :
      for( i=0; i<VECTOR_SIZE32( vec->width ); i++ ) {
        uint32 value_a = vec->value.u32[i][VTYPE_INDEX_EXP_EVAL_A]; 
        uint32 value_b = vec->value.u32[i][VTYPE_INDEX_EXP_EVAL_B]; 
        uint32 value_c = vec->value.u32[i][VTYPE_INDEX_EXP_EVAL_C]; 
        for( j=0; j<32; j++ ) {
          count += (value_a >> j) & 0x1;
          count += (value_b >> j) & 0x1;
          count += (value_c >> j) & 0x1;
        }
      }
      break;
    default :  assert( 0 );  break;
  }
  
  PROFILE_END;

  return( count );

}

/*!
 \param vec  Pointer to vector to count eval_a/b/c/d bits in

 \return Returns the number of eval_a/b/c/d bits are set in the given vector.
*/
int vector_get_eval_abcd_count(
  vector* vec
) { PROFILE(VECTOR_GET_EVAL_ABCD_COUNT);

  int          count = 0;  /* Number of EVAL_A/B/C/D bits set */
  unsigned int i, j;       /* Loop iterators */

  switch( vec->suppl.part.data_type ) {
    case VDATA_U32 :
      for( i=0; i<VECTOR_SIZE32( vec->width ); i++ ) {
        uint32 value_a = vec->value.u32[i][VTYPE_INDEX_EXP_EVAL_A]; 
        uint32 value_b = vec->value.u32[i][VTYPE_INDEX_EXP_EVAL_B]; 
        uint32 value_c = vec->value.u32[i][VTYPE_INDEX_EXP_EVAL_C]; 
        uint32 value_d = vec->value.u32[i][VTYPE_INDEX_EXP_EVAL_D]; 
        for( j=0; j<32; j++ ) {
          count += (value_a >> j) & 0x1;
          count += (value_b >> j) & 0x1;
          count += (value_c >> j) & 0x1;
          count += (value_d >> j) & 0x1;
        }
      }
      break;
    default :  assert( 0 );  break;
  }
  
  PROFILE_END;

  return( count );

}

/*!
 \param nib    Pointer to vector data array to get string from
 \param width  Width of given vector data array

 \return Returns a string showing the toggle 0 -> 1 information.
*/
char* vector_get_toggle01_uint32(
  uint32** value,
  int      width
) { PROFILE(VECTOR_GET_TOGGLE01_UINT32);

  char* bits      = (char*)malloc_safe( width + 1 );
  int   bits_left = ((width - 1) & 0x1f);
  int   i, j;
  char  tmp[2];

  for( i=VECTOR_SIZE32(width); i--; ) {
    for( j=bits_left; j>=0; j-- ) {
      /*@-formatcode@*/
      unsigned int rv = snprintf( tmp, 2, "%hhx", (unsigned char)((value[i][VTYPE_INDEX_SIG_TOG01] >> j) & 0x1) );
      /*@=formatcode@*/
      assert( rv < 2 );
      bits[((width - 1) - i)] = tmp[0];
    }
    bits_left = 31;
  }

  bits[width] = '\0';

  PROFILE_END;

  return( bits );

}

/*!
 \param nib    Pointer to vector data array to get string from
 \param width  Width of given vector data array

 \return Returns a string showing the toggle 1 -> 0 information.
*/
char* vector_get_toggle10_uint32(
  uint32** value,
  int      width
) { PROFILE(VECTOR_GET_TOGGLE10_UINT32);

  char* bits      = (char*)malloc_safe( width + 1 );
  int   bits_left = ((width - 1) & 0x1f);
  int   i, j;
  char  tmp[2];
  
  for( i=VECTOR_SIZE32(width); i--; ) {
    for( j=bits_left; j>=0; j-- ) {
      /*@-formatcode@*/ 
      unsigned int rv = snprintf( tmp, 2, "%hhx", (unsigned char)((value[i][VTYPE_INDEX_SIG_TOG10] >> j) & 0x1) );
      /*@=formatcode@*/ 
      assert( rv < 2 );
      bits[((width - 1) - i)] = tmp[0];
    } 
    bits_left = 31;
  } 
  
  bits[width] = '\0';

  PROFILE_END;

  return( bits );

}

/*!
 \param nib    Nibble to display toggle information
 \param width  Number of bits of nibble to display
 \param ofile  Stream to output information to.
 
 Displays the toggle01 information from the specified vector to the output
 stream specified in ofile.
*/
void vector_display_toggle01_uint32(
  uint32** value,
  int      width,
  FILE*    ofile
) { PROFILE(VECTOR_DISPLAY_TOGGLE01_UINT32);

  unsigned int nib       = 0;
  int          i, j;
  int          bits_left = ((width - 1) & 0x1f);

  fprintf( ofile, "%d'h", width );

  for( i=VECTOR_SIZE32(width); i--; ) {
    for( j=bits_left; j>=0; j-- ) {
      nib |= (((value[i][VTYPE_INDEX_SIG_TOG01] >> j) & 0x1) << ((unsigned)j % 4));
      if( (j % 4) == 0 ) {
        fprintf( ofile, "%1x", nib );
        nib = 0;
      }
      if( ((j % 16) == 0) && ((j != 0) || (i != 0)) ) {
        fprintf( ofile, "_" );
      }
    }
    bits_left = 31;
  }

  PROFILE_END;

}

/*!
 \param nib    Nibble to display toggle information
 \param width  Number of bits of nibble to display
 \param ofile  Stream to output information to.
 
 Displays the toggle10 information from the specified vector to the output
 stream specified in ofile.
*/
void vector_display_toggle10_uint32(
  uint32** value,
  int      width,
  FILE*    ofile
) { PROFILE(VECTOR_DISPLAY_TOGGLE10_UINT32);

  unsigned int nib       = 0;
  int          i, j;
  int          bits_left = ((width - 1) & 0x1f);
  
  fprintf( ofile, "%d'h", width );
      
  for( i=VECTOR_SIZE32(width); i--; ) {
    for( j=bits_left; j>=0; j-- ) {
      nib |= (((value[i][VTYPE_INDEX_SIG_TOG10] >> j) & 0x1) << ((unsigned)j % 4));
      if( (j % 4) == 0 ) {
        fprintf( ofile, "%1x", nib );
        nib = 0;
      }
      if( ((j % 16) == 0) && ((j != 0) || (i != 0)) ) {
        fprintf( ofile, "_" );
      }
    } 
    bits_left = 31;
  }

  PROFILE_END;

}

/*!
 \param nib    Pointer to vector nibble array
 \param width  Number of elements in nib array

 Displays the binary value of the specified vector data array to standard output.
*/
void vector_display_value_uint32(
  uint32** value,
  int      width
) {

  int i, j;  /* Loop iterator */
  int bits_left = ((width - 1) & 0x1f);

  printf( "value: %d'b", width );

  for( i=VECTOR_SIZE32(width); i--; ) {
    for( j=bits_left; j>=0; j-- ) {
      if( ((value[i][VTYPE_INDEX_VAL_VALH] >> j) & 0x1) == 0 ) {
        printf( "%d", ((value[i][VTYPE_INDEX_VAL_VALL] >> j) & 0x1) );
      } else {
        if( ((value[i][VTYPE_INDEX_VAL_VALL] >> j) & 0x1) == 0 ) {
          printf( "x" );
        } else {
          printf( "z" );
        }
      }
    }
    bits_left = 31;
  }

}

/*!
 \param nib    Nibble to display
 \param width  Number of bits in nibble to display
 \param type   Type of vector to display

 Outputs the specified nibble array to standard output as described by the
 width parameter.
*/
void vector_display_nibble_uint32(
  uint32** value,
  int      width,
  int      type
) {

  int i, j;  /* Loop iterator */

  for( i=0; i<vector_type_sizes[type]; i++ ) {
    for( j=VECTOR_SIZE32(width); j--; ) {
      /*@-formatcode@*/
      printf( " %x", value[j][i] );
      /*@=formatcode@*/
    }
  }

  /* Display nibble value */
  printf( ", " );
  vector_display_value_uint32( value, width );

  switch( type ) {

    case VTYPE_SIG :

      /* Display nibble toggle01 history */
      printf( ", 0->1: " );
      vector_display_toggle01_uint32( value, width, stdout );

      /* Display nibble toggle10 history */
      printf( ", 1->0: " );
      vector_display_toggle10_uint32( value, width, stdout );

      /* Display bit set information */
      printf( ", set: %d'h", width );
      for( i=VECTOR_SIZE32(width); i--; ) {
        /*@-formatcode@*/
        printf( "%x", value[i][VTYPE_INDEX_SIG_SET] );
        /*@=formatcode@*/
      }

      break;

    case VTYPE_EXP :

      /* Display eval_a information */
      printf( ", a: %d'h", width );
      for( i=VECTOR_SIZE32(width); i--; ) {
        /*@-formatcode@*/
        printf( "%x", value[i][VTYPE_INDEX_EXP_EVAL_A] );
        /*@=formatcode@*/
      }

      /* Display eval_b information */
      printf( ", b: %d'h", width );
      for( i=VECTOR_SIZE32(width); i--; ) {
        /*@-formatcode@*/
        printf( "%x", value[i][VTYPE_INDEX_EXP_EVAL_B] );
        /*@=formatcode@*/
      }

      /* Display eval_c information */
      printf( ", c: %d'h", width );
      for( i=VECTOR_SIZE32(width); i--; ) {
        /*@-formatcode@*/
        printf( "%x", value[i][VTYPE_INDEX_EXP_EVAL_C] );
        /*@=formatcode@*/
      }

      /* Display eval_d information */
      printf( ", d: %d'h", width );
      for( i=VECTOR_SIZE32(width); i--; ) {
        /*@-formatcode@*/
        printf( "%x", value[i][VTYPE_INDEX_EXP_EVAL_D] );
        /*@=formatcode@*/
      }

      /* Display set information */
      printf( ", set: %d'h", width );
      for( i=VECTOR_SIZE32(width); i--; ) {
        /*@-formatcode@*/
        printf( "%x", value[i][VTYPE_INDEX_EXP_SET] );
        /*@=formatcode@*/
      }

      break;

    case VTYPE_MEM :
  
      /* Display nibble toggle01 history */
      printf( ", 0->1: " );
      vector_display_toggle01_uint32( value, width, stdout );

      /* Display nibble toggle10 history */
      printf( ", 1->0: " );
      vector_display_toggle10_uint32( value, width, stdout );

      /* Write history */
      printf( ", wr: %d'h", width );
      for( i=VECTOR_SIZE32(width); i--; ) {
        /*@-formatcode@*/
        printf( "%x", value[i][VTYPE_INDEX_MEM_WR] );
        /*@=formatcode@*/
      }

      /* Read history */
      printf( ", rd: %d'h", width );
      for( i=VECTOR_SIZE32(width); i--; ) {
        /*@-formatcode@*/
        printf( "%x", value[i][VTYPE_INDEX_MEM_RD] );
        /*@=formatcode@*/
      }

      break;

    default : break;

  }

}

/*!
 \param vec  Pointer to vector to output to standard output.

 Outputs contents of vector to standard output (for debugging purposes only).
*/
void vector_display(
  const vector* vec
) {

  assert( vec != NULL );

  /*@-formatcode@*/
  printf( "Vector (%p) => width: %d, suppl: %hhx\n", vec, vec->width, vec->suppl.all );
  /*@=formatcode@*/

  if( (vec->width > 0) && (vec->value.u32 != NULL) ) {
    switch( vec->suppl.part.data_type ) {
      case VDATA_U32 :  vector_display_nibble_uint32( vec->value.u32, vec->width, vec->suppl.part.type );  break;
      default        :  assert( 0 );  break;
    }
  } else {
    printf( "NO DATA" );
  }

  printf( "\n" );

}

/*!
 \param vec        Pointer to vector to parse.
 \param tog01_cnt  Number of bits in vector that toggled from 0 to 1.
 \param tog10_cnt  Number of bits in vector that toggled from 1 to 0.

 Walks through specified vector counting the number of toggle01 bits that
 are set and the number of toggle10 bits that are set.  Adds these values
 to the contents of tog01_cnt and tog10_cnt.
*/
void vector_toggle_count(
  vector* vec,
  int*    tog01_cnt,
  int*    tog10_cnt
) { PROFILE(VECTOR_TOGGLE_COUNT);

  if( (vec->suppl.part.type == VTYPE_SIG) || (vec->suppl.part.type == VTYPE_MEM) ) {

    unsigned int i, j;

    switch( vec->suppl.part.data_type ) {
      case VDATA_U32 :
        for( i=0; i<VECTOR_SIZE32(vec->width); i++ ) {
          for( j=0; j<32; j++ ) {
            *tog01_cnt += ((vec->value.u32[i][VTYPE_INDEX_SIG_TOG01] >> j) & 0x1);
            *tog10_cnt += ((vec->value.u32[i][VTYPE_INDEX_SIG_TOG10] >> j) & 0x1);
          }
        }
        break;
      default :  assert( 0 );  break;
    }

  }

  PROFILE_END;

}

/*!
 \param vec     Pointer to vector to parse
 \param wr_cnt  Pointer to number of bits in vector that were written
 \param rd_cnt  Pointer to number of bits in vector that were read

 Counts the number of bits that were written and read for the given memory
 vector.
*/
void vector_mem_rw_count(
  vector* vec,
  int*    wr_cnt,
  int*    rd_cnt
) { PROFILE(VECTOR_MEM_RW_COUNT);

  unsigned int i, j;  /* Loop iterator */

  switch( vec->suppl.part.data_type ) {
    case VDATA_U32 :
      for( i=0; i<VECTOR_SIZE32(vec->width); i++ ) {
        for( j=0; j<32; j++ ) {
          *wr_cnt += vec->value.u32[i][VTYPE_INDEX_MEM_WR];
          *rd_cnt += vec->value.u32[i][VTYPE_INDEX_MEM_RD];
        }
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

}

/*!
 \param vec  Pointer to vector value to set
 \param msb  Most-significant bit to set in vector value
 \param lsb  Least-significant bit to set in vector value

 \return Returns TRUE if assigned bit that is being set to 1 in this function was
         found to be previously set; otherwise, returns FALSE.

 Sets the assigned supplemental bit for the given bit range in the given vector.  Called by
 race condition checker code.
*/
bool vector_set_assigned(
  vector* vec,
  int     msb,
  int     lsb
) { PROFILE(VECTOR_SET_ASSIGNED);

  bool          prev_assigned = FALSE;  /* Specifies if any set bit was previously set */
  unsigned int  i;                      /* Loop iterator */

  assert( vec != NULL );
  assert( (msb - lsb) < vec->width );
  assert( vec->suppl.part.type == VTYPE_SIG );

  switch( vec->suppl.part.data_type ) {
    case VDATA_U32 :
      for( i=lsb; i<=msb; i++ ) {
        unsigned int offset = (i >> 5);
        if( ((vec->value.u32[offset][VTYPE_INDEX_SIG_MISC] >> (i & 0x1f)) & 0x1) == 1 ) {
          prev_assigned = TRUE;
        }
        vec->value.u32[offset][VTYPE_INDEX_SIG_MISC] |= (0x1 << (i & 0x1f));
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( prev_assigned );

}

/*!
 \param vec  Pointer to vector to calculate coverage metrics for and perform scratch -> actual assignment
 \param scratchl  Pointer to scratch array containing new lower data
 \param scratchh  Pointer to scratch array containing new upper data
 \param lsb  Least-significant bit to get coverage for
 \param msb  Most-significant bit to get coverage for

 \return Returns TRUE if the assigned value has changed; otherwise, returns FALSE.

 This function is called after a value has been stored in the SCRATCH arrays.  This
 function calculates the vector coverage information based on the vector type and performs the assignment
 from the SCRATCH array to the 
*/
bool vector_set_coverage_and_assign_uint32(
  vector*       vec,
  const uint32* scratchl,
  const uint32* scratchh,
  int           lsb,
  int           msb
) { PROFILE(VECTOR_SET_COVERAGE_AND_ASSIGN);

  bool         changed = FALSE;                              /* Set to TRUE if the assigned value has changed */
  unsigned int lindex  = (lsb >> 5);                         /* Index of lowest array entry */
  unsigned int hindex  = (msb >> 5);                         /* Index of highest array entry */
  uint32       lmask   = (0xffffffff << lsb);                /* Mask to be used in lower element */
  uint32       hmask   = (0xffffffff >> (31-(msb & 0x1f)));  /* Mask to be used in upper element */
  unsigned int i;                                            /* Loop iterator */

  /* If the lindex and hindex are the same, set lmask to the AND of the high and low masks */
  if( lindex == hindex ) {
    lmask &= hmask;
  }

  switch( vec->suppl.part.type ) {
    case VTYPE_VAL :
      for( i=lindex; i<=hindex; i++ ) {
        uint32* tvall = &(vec->value.u32[i][VTYPE_INDEX_SIG_VALL]);
        uint32* tvalh = &(vec->value.u32[i][VTYPE_INDEX_SIG_VALH]);
        uint32  mask  = (i==lindex) ? lmask : (i==hindex ? hmask : 0xffffffff);
        *tvall = (*tvall & ~mask) | (scratchl[i] & mask);
        *tvalh = (*tvalh & ~mask) | (scratchh[i] & mask);
      }
      changed = TRUE;
      break;
    case VTYPE_SIG :
      for( i=lindex; i<=hindex; i++ ) {
        uint32* entry = vec->value.u32[i];
        uint32  mask  = (i==lindex) ? lmask : (i==hindex ? hmask : 0xffffffff);
        uint32  fvall = scratchl[i] & mask;
        uint32  fvalh = scratchh[i] & mask;
        uint32  tvall = entry[VTYPE_INDEX_SIG_VALL];
        uint32  tvalh = entry[VTYPE_INDEX_SIG_VALH];
        uint32  set   = entry[VTYPE_INDEX_SIG_SET];
        if( (fvall != (tvall & mask)) || (fvalh != (tvalh & mask)) || ((set & mask) == 0) ) {
          if( (set & mask) != 0 ) {
            uint32 cmp = ((~fvalh & fvall) ^ ((~tvalh & tvall) & mask));
            entry[VTYPE_INDEX_SIG_TOG01] |= cmp & (~fvalh &  fvall);
            entry[VTYPE_INDEX_SIG_TOG10] |= cmp & ( fvalh | ~fvall);
          }
          entry[VTYPE_INDEX_SIG_SET] |= mask;
          entry[VTYPE_INDEX_SIG_VALL] = (tvall & ~mask) | fvall;
          entry[VTYPE_INDEX_SIG_VALH] = (tvalh & ~mask) | fvalh;
          changed = TRUE;
        }
      }
      break;
    case VTYPE_MEM :
      for( i=lindex; i<=hindex; i++ ) {
        uint32* entry = vec->value.u32[i];
        uint32  mask  = (i==lindex) ? lmask : (i==hindex ? hmask : 0xffffffff);
        uint32  fvall = scratchl[i] & mask;
        uint32  fvalh = scratchh[i] & mask;
        uint32  tvall = entry[VTYPE_INDEX_MEM_VALL];
        uint32  tvalh = entry[VTYPE_INDEX_MEM_VALH];
        if( (fvall != (tvall & mask)) || (fvalh != (tvalh & mask)) ) {
          uint32 cmp = ((~fvalh & fvall) ^ ((~tvalh & tvall) & mask));
          entry[VTYPE_INDEX_MEM_TOG01] |= cmp & ~(~fvalh & fvall);
          entry[VTYPE_INDEX_MEM_TOG10] |= cmp &  (~fvalh & fvall);
          entry[VTYPE_INDEX_MEM_WR]     = mask;
          entry[VTYPE_INDEX_MEM_VALL]   = (tvall & ~mask) | fvall;
          entry[VTYPE_INDEX_MEM_VALH]   = (tvalh & ~mask) | fvalh;
          changed = TRUE;
        }
      }
      break;
    case VTYPE_EXP :
      for( i=lindex; i<=hindex; i++ ) {
        uint32* entry = vec->value.u32[i];
        uint32  mask  = (i==lindex) ? lmask : (i==hindex ? hmask : 0xffffffff);
        uint32  fvall = scratchl[i] & mask;
        uint32  fvalh = scratchh[i] & mask;
        uint32  tvall = entry[VTYPE_INDEX_EXP_VALL];
        uint32  tvalh = entry[VTYPE_INDEX_EXP_VALH];
        uint32  set   = entry[VTYPE_INDEX_EXP_SET];
        if( (fvall != (tvall & mask)) || (fvalh != (tvalh & mask)) || ((set & mask) == 0) ) {
          entry[VTYPE_INDEX_EXP_SET] |= mask;
          entry[VTYPE_INDEX_EXP_VALL] = (tvall & ~mask) | fvall;
          entry[VTYPE_INDEX_EXP_VALH] = (tvalh & ~mask) | fvalh;
          changed = TRUE;
        }
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( changed );

}

/*!
 Performs a fast left-shift of 0-bit-aligned data in vector and stores the result in the vall/h
 value arrays.
*/
static void vector_lshift_uint32(
            vector*       vec,    /*!< Pointer to vector containing value that we want to left-shift */
            uint32*       vall,   /*!< Pointer to intermediate value array containing lower bits of shifted value */
            uint32*       valh,   /*!< Pointer to intermediate value array containing upper bits of shifted value */
            unsigned int  shift,  /*!< Number of bits to shift */
  /*@out@*/ unsigned int* msb     /*!< Pointer to calculated MSB */
) { PROFILE(VECTOR_LSHIFT_UINT32);

  unsigned int diff;

  *msb = ((vec->width + shift) - 1);
  diff = (*msb >> 5) - ((vec->width - 1) >> 5);

  if( (shift >> 5) == (*msb >> 5) ) {

    vall[0] = (vec->value.u32[0][VTYPE_INDEX_VAL_VALL] << shift);
    valh[0] = (vec->value.u32[0][VTYPE_INDEX_VAL_VALH] << shift);

  } else if( (shift & 0x1f) == 0 ) {

    int i;

    for( i=((vec->width >> 5) - 1); i>=0; i-- ) {
      vall[i+diff] = vec->value.u32[i][VTYPE_INDEX_VAL_VALL];
      valh[i+diff] = vec->value.u32[i][VTYPE_INDEX_VAL_VALH];
    }

    for( i=((shift >> 5)-1); i>=0; i-- ) {
      vall[i] = 0;
      valh[i] = 0;
    }

  } else if( (*msb & 0x1f) > ((vec->width - 1) & 0x1f) ) {

    unsigned int mask_bits1  = (vec->width & 0x1f);
    unsigned int shift_bits1 = (*msb & 0x1f) - ((vec->width - 1) & 0x1f);
    uint32       mask1       = 0xffffffff >> (32 - mask_bits1);
    uint32       mask2       = 0xffffffff << (32 - shift_bits1);
    uint32       mask3       = ~mask2;
    int          i;

    vall[*msb>>5] = (vec->value.u32[(vec->width-1)>>5][VTYPE_INDEX_VAL_VALL] & mask1) << shift_bits1;
    valh[*msb>>5] = (vec->value.u32[(vec->width-1)>>5][VTYPE_INDEX_VAL_VALH] & mask1) << shift_bits1;

    for( i=(((vec->width - 1) >> 5) - 1); i>=0; i-- ) {
      vall[i+diff+1] |= ((vec->value.u32[i][VTYPE_INDEX_VAL_VALL] & mask2) >> (32 - shift_bits1));
      valh[i+diff+1] |= ((vec->value.u32[i][VTYPE_INDEX_VAL_VALH] & mask2) >> (32 - shift_bits1));
      vall[i+diff]    = ((vec->value.u32[i][VTYPE_INDEX_VAL_VALL] & mask3) << shift_bits1);
      valh[i+diff]    = ((vec->value.u32[i][VTYPE_INDEX_VAL_VALH] & mask3) << shift_bits1);
    }

    for( i=((shift >> 5)-1); i>=0; i-- ) {
      vall[i] = 0;
      valh[i] = 0;
    }

  } else {

    unsigned int mask_bits1  = ((vec->width - 1) & 0x1f);
    unsigned int shift_bits1 = mask_bits1 - (*msb & 0x1f);
    uint32       mask1       = 0xffffffff << mask_bits1;
    uint32       mask2       = 0xffffffff >> (32 - shift_bits1);
    uint32       mask3       = ~mask2;
    int          i;

    vall[*msb>>5] = (vec->value.u32[(vec->width-1)>>5][VTYPE_INDEX_VAL_VALL] & mask1) >> shift_bits1;
    valh[*msb>>5] = (vec->value.u32[(vec->width-1)>>5][VTYPE_INDEX_VAL_VALH] & mask1) >> shift_bits1;

    for( i=((vec->width - 1) >> 5); i>=0; i-- ) {
      vall[(i+diff)-1]  = ((vec->value.u32[i][VTYPE_INDEX_VAL_VALL] & mask2) << (32 - shift_bits1));
      valh[(i+diff)-1]  = ((vec->value.u32[i][VTYPE_INDEX_VAL_VALH] & mask2) << (32 - shift_bits1));
      if( i > 0 ) {
        vall[(i+diff)-1] |= ((vec->value.u32[i-1][VTYPE_INDEX_VAL_VALL] & mask3) >> shift_bits1);
        valh[(i+diff)-1] |= ((vec->value.u32[i-1][VTYPE_INDEX_VAL_VALH] & mask3) >> shift_bits1);
      }
    }

    for( i=((shift >> 5)-1); i>=0; i-- ) {
      vall[i] = 0;
      valh[i] = 0;
    }

  }

  PROFILE_END;

}

/*!
 \param vec    Pointer to vector to set value to.
 \param value  New value to set vector value to.
 \param width  Width of new value.

 \return Returns TRUE if assignment was performed; otherwise, returns FALSE.

 Allows the calling function to set any bit vector within the vector
 range.  If the vector value has never been set, sets
 the value to the new value and returns.  If the vector value has previously
 been set, checks to see if new vector bits have toggled, sets appropriate
 toggle values, sets the new value to this value and returns.
*/
bool vector_set_value_uint32(
  vector*   vec,
  uint32**  value,
  int       width
) { PROFILE(VECTOR_SET_VALUE);

  bool   retval = FALSE;                /* Return value for this function */
  int    size;                          /* Size of vector value array */
  int    i;                             /* Loop iterator */
  int    v2st;                          /* Value to AND with from value bit if the target is a 2-state value */
  uint32 scratchl[MAX_BIT_WIDTH >> 5];  /* Lower scratch array */
  uint32 scratchh[MAX_BIT_WIDTH >> 5];  /* Upper scratch array */

  assert( vec != NULL );

  /* Adjust the width if it exceeds our width */
  if( vec->width < width ) {
    width = vec->width;
  }

  /* Get some information from the vector */
  v2st = vec->suppl.part.is_2state << 1;
  size = VECTOR_SIZE32( width );

  /* Calculate the new values and place them in the scratch arrays */
  for( i=size; i--; ) {
    scratchl[i] = v2st ? (~value[i][VTYPE_INDEX_VAL_VALH] & value[i][VTYPE_INDEX_VAL_VALL]) : value[i][VTYPE_INDEX_VAL_VALL];
    scratchh[i] = v2st ? 0 : value[i][VTYPE_INDEX_VAL_VALH];
  }

  /* Calculate the coverage and perform the actual assignment */
  retval = vector_set_coverage_and_assign_uint32( vec, scratchl, scratchh, 0, (width - 1) );

  PROFILE_END;

  return( retval );

}

/*!
 \return Returns TRUE if stored data differed from original data; otherwise, returns FALSE.

 Used for single- and multi-bit part selection.  Bits are pulled from the source vector via the
 LSB and MSB range
*/
bool vector_part_select(
  vector* tgt,        /*!< Pointer to vector that will store the result */
  vector* src,        /*!< Pointer to vector containing data to store */
  int     lsb,        /*!< LSB offset */
  int     msb,        /*!< MSB offset */
  bool    set_mem_rd  /*!< If TRUE, set the memory read bit in the source */
) { PROFILE(VECTOR_PART_SELECT);

  bool retval;  /* Return value for this function */

  switch( src->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        uint32       valh[MAX_BIT_WIDTH>>5];
        uint32       vall[MAX_BIT_WIDTH>>5];
        int          i;
        unsigned int pos = 0;

        assert( ((msb-lsb) + 1) <= tgt->width );

        for( i=lsb; i<=msb; i++ ) {
          uint32*      entry    = src->value.u32[i>>5];
          unsigned int my_index = (pos >> 5);
          unsigned int offset   = (i & 0x1f);
          if( (pos & 0x1f) == 0 ) {
            valh[my_index] = 0;
            vall[my_index] = 0;
          }
          vall[my_index] |= ((entry[VTYPE_INDEX_VAL_VALL] >> offset) & 0x1) << pos;
          valh[my_index] |= ((entry[VTYPE_INDEX_VAL_VALH] >> offset) & 0x1) << pos;
          if( set_mem_rd ) {
            entry[VTYPE_INDEX_MEM_RD] |= (1 << offset);
          }
          pos++;
        }
        retval = vector_set_coverage_and_assign_uint32( tgt, vall, valh, lsb, msb );
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \param vec  Pointer to vector to set eval_a/b bits for unary operation

 Called by expression_op_func__* functions for operations that are unary
 in nature.
*/
void vector_set_unary_evals(
  vector* vec
) { PROFILE(VECTOR_SET_UNARY_EVALS);

  switch( vec->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        unsigned int i;
        for( i=0; i<VECTOR_SIZE32(vec->width); i++ ) {
          uint32* entry = vec->value.u32[i];
          uint32  lval  = entry[VTYPE_INDEX_EXP_VALL];
          uint32  nhval = ~entry[VTYPE_INDEX_EXP_VALH];
          entry[VTYPE_INDEX_EXP_EVAL_A] = nhval & ~lval;
          entry[VTYPE_INDEX_EXP_EVAL_B] = nhval &  lval;
        }
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

}

/*!
 \param tgt    Pointer to target vector to set eval_a/b/c supplemental bits
 \param left   Pointer to target vector on the left
 \param right  Pointer to target vector on the right

 Sets the eval_a/b/c supplemental bits as necessary.  This function should be called
 by expression_op_func__* functions that are AND-type combinational operations only and own their
 own vectors.
*/    
void vector_set_and_comb_evals(
  vector* tgt,
  vector* left,
  vector* right
) { PROFILE(VECTOR_SET_AND_COMB_EVALS);

  switch( tgt->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        unsigned int i;
        unsigned int size  = VECTOR_SIZE32( tgt->width );
        unsigned int lsize = VECTOR_SIZE32( left->width );
        unsigned int rsize = VECTOR_SIZE32( right->width );

        for( i=0; i<size; i++ ) {
          uint32* val    = tgt->value.u32[i];
          uint32* lval   = left->value.u32[i];
          uint32* rval   = right->value.u32[i];
          uint32  lvall  = (i < lsize) ? lval[VTYPE_INDEX_EXP_VALL] : 0;
          uint32  nlvalh = (i < lsize) ? lval[VTYPE_INDEX_EXP_VALH] : 0xffffffff;
          uint32  rvall  = (i < rsize) ? rval[VTYPE_INDEX_EXP_VALL] : 0;
          uint32  nrvalh = (i < rsize) ? rval[VTYPE_INDEX_EXP_VALH] : 0xffffffff;
          
          val[VTYPE_INDEX_EXP_EVAL_A] |= nlvalh & ~lvall;
          val[VTYPE_INDEX_EXP_EVAL_B] |= nrvalh & ~rvall;
          val[VTYPE_INDEX_EXP_EVAL_C] |= nlvalh & nrvalh & lvall & rvall;
        }
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;
 
}

/*!
 \param tgt    Pointer to target vector to set eval_a/b/c supplemental bits
 \param left   Pointer to vector on left of expression
 \param right  Pointer to vector on right of expression

 Sets the eval_a/b/c supplemental bits as necessary.  This function should be called
 by expression_op_func__* functions that are OR-type combinational operations only and own their
 own vectors.
*/
void vector_set_or_comb_evals(
  vector* tgt,
  vector* left,
  vector* right
) { PROFILE(VECTOR_SET_OR_COMB_EVALS);

  switch( tgt->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        unsigned int i;
        unsigned int size  = VECTOR_SIZE32( tgt->width );
        unsigned int lsize = VECTOR_SIZE32( left->width );
        unsigned int rsize = VECTOR_SIZE32( right->width );

        for( i=0; i<size; i++ ) {
          uint32* val    = tgt->value.u32[i];
          uint32* lval   = left->value.u32[i];
          uint32* rval   = right->value.u32[i];
          uint32  lvall  = (i < lsize) ? lval[VTYPE_INDEX_EXP_VALL] : 0;
          uint32  nlvalh = (i < lsize) ? lval[VTYPE_INDEX_EXP_VALH] : 0;
          uint32  rvall  = (i < rsize) ? rval[VTYPE_INDEX_EXP_VALL] : 0;
          uint32  nrvalh = (i < rsize) ? rval[VTYPE_INDEX_EXP_VALH] : 0;

          val[VTYPE_INDEX_EXP_EVAL_A] |= nlvalh & lvall;
          val[VTYPE_INDEX_EXP_EVAL_B] |= nrvalh & rvall;
          val[VTYPE_INDEX_EXP_EVAL_C] |= nlvalh & nrvalh & ~lvall & ~rvall;
        }
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

}

/*! 
 \param tgt    Pointer to target vector to set eval_a/b/c/d supplemental bits
 \param left   Pointer to vector on left of expression
 \param right  Pointer to vector on right of expression

 Sets the eval_a/b/c/d supplemental bits as necessary.  This function should be called
 by expression_op_func__* functions that are OTHER-type combinational operations only and own their
 own vectors. 
*/
void vector_set_other_comb_evals(
  vector* tgt,
  vector* left,
  vector* right
) { PROFILE(VECTOR_SET_OTHER_COMB_EVALS);

  switch( tgt->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        unsigned int i;
        unsigned int size  = VECTOR_SIZE32( tgt->width );
        unsigned int lsize = VECTOR_SIZE32( left->width );
        unsigned int rsize = VECTOR_SIZE32( right->width );

        for( i=0; i<size; i++ ) { 
          uint32* val    = tgt->value.u32[i];
          uint32* lval   = left->value.u32[i];
          uint32* rval   = right->value.u32[i];
          uint32  lvall  = (i < lsize) ? lval[VTYPE_INDEX_EXP_VALL] : 0;
          uint32  nlvalh = (i < lsize) ? lval[VTYPE_INDEX_EXP_VALH] : 0;
          uint32  rvall  = (i < rsize) ? rval[VTYPE_INDEX_EXP_VALL] : 0;
          uint32  nrvalh = (i < rsize) ? rval[VTYPE_INDEX_EXP_VALH] : 0;
          uint32  nvalh  = nlvalh & nrvalh;

          val[VTYPE_INDEX_EXP_EVAL_A] |= nvalh & ~lvall & ~rvall;
          val[VTYPE_INDEX_EXP_EVAL_B] |= nvalh & ~lvall &  rvall;
          val[VTYPE_INDEX_EXP_EVAL_C] |= nvalh &  lvall & ~rvall;
          val[VTYPE_INDEX_EXP_EVAL_D] |= nvalh &  lvall &  rvall;
        }
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;
  
} 

/*!
 \param vec   Pointer to vector to bit-fill
 \param last  Bit position of last set value

 \return Returns TRUE if any of the bits in the bit-fill range have changed
         value.

 Performs a bit-fill of the specified vector starting at the specified LSB
 and bit-filling all bits to the MSB.
*/
bool vector_bit_fill(
  vector* vec,
  int     last
) { PROFILE(VECTOR_BIT_FILL);

  bool     changed = FALSE;             /* Return value for this function */
  int      i;                           /* Loop iterator */

  assert( vec != NULL );
  assert( last > 0 );
  assert( last < vec->width ); 

  switch( vec->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        unsigned int last_index = (last >> 5);
        unsigned int next_index = ((last + 1) >> 5);
        unsigned int bit_pos    = (last & 0x1f);
        uint32       valh       = vec->value.u32[last_index][VTYPE_INDEX_VAL_VALH];
        uint32       mask       = 0xffffffff << (bit_pos + 1);
        uint32       scratchl[MAX_BIT_WIDTH >> 5];
        uint32       scratchh[MAX_BIT_WIDTH >> 5];
        if( (valh & (1 << bit_pos)) == 0 ) {
          for( i=next_index; i<VECTOR_SIZE32(vec->width); i++ ) {
            uint32* entry = vec->value.u32[i];
            scratchl[i] = (entry[VTYPE_INDEX_VAL_VALL] & ~mask);
            scratchh[i] = (entry[VTYPE_INDEX_VAL_VALH] & ~mask);
            mask        = 0xffffffff;
          }
        } else {
          uint32 vall = vec->value.u32[last_index][VTYPE_INDEX_VAL_VALL];
          if( (vall & (1 << bit_pos)) == 0 ) {
            for( i=next_index; i<VECTOR_SIZE32(vec->width); i++ ) {
              uint32* entry = vec->value.u32[i];
              scratchl[i] = (entry[VTYPE_INDEX_VAL_VALL] & ~mask);
              scratchh[i] = (entry[VTYPE_INDEX_VAL_VALH] & ~mask) | mask;
              mask        = 0xffffffff;
            }
          } else {
            for( i=next_index; i<VECTOR_SIZE32(vec->width); i++ ) {
              uint32* entry = vec->value.u32[i];
              scratchl[i] = (entry[VTYPE_INDEX_VAL_VALL] & ~mask) | mask;
              scratchh[i] = (entry[VTYPE_INDEX_VAL_VALH] & ~mask) | mask;
              mask        = 0xffffffff;
            }
          }
        }

        /* Get coverage information and perform assign */
        vector_set_coverage_and_assign_uint32( vec, scratchl, scratchh, (last + 1), (vec->width - 1) );
      }
      break;
    default :  assert( 0 );  break;
  }
 
  return( changed );

}

/*!
 \param vec  Pointer to vector check for unknown-ness

 \return Returns TRUE if the given vector contains unknown (X or Z) bits; otherwise, returns FALSE.
*/
bool vector_is_unknown(
  const vector* vec
) { PROFILE(VECTOR_IS_UKNOWN);

  unsigned int i = 0;  /* Loop iterator */
  unsigned int size;   /* Size of data array */

  assert( vec != NULL );
  assert( vec->value.u32 != NULL );

  switch( vec->suppl.part.data_type ) {
    case VDATA_U32 :
      size = VECTOR_SIZE32( vec->width );
      while( (i < size) && (vec->value.u32[i][VTYPE_INDEX_VAL_VALH] == 0) ) i++;
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( i < size );

}

/*!
 \param vec  Pointer to vector to check for non-zero-ness

 \return Returns TRUE if the given vector contains at least one non-zero bit; otherwise, returns FALSE.
*/
bool vector_is_not_zero(
  const vector* vec
) { PROFILE(VECTOR_IS_NOT_ZERO);

  unsigned int i = 0;  /* Loop iterator */
  unsigned int size;   /* Size of data array */

  assert( vec != NULL );
  assert( vec->value.u32 != NULL );

  switch( vec->suppl.part.data_type ) {
    case VDATA_U32 :
      size = VECTOR_SIZE32( vec->width );
      while( (i < size) && (vec->value.u32[i][VTYPE_INDEX_VAL_VALL] == 0) ) i++;
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( i < size );

}

/*!
 \param vec  Pointer to vector to set to a value of X

 \return Returns TRUE if the given vector changed value; otherwise, returns FALSE.

 Sets the entire specified vector to a value of X.
*/
bool vector_set_to_x(
  vector* vec
) { PROFILE(VECTOR_SET_TO_X);

  bool retval;  /* Return value for this function */

  switch( vec->suppl.part.data_type ) {
    case VDATA_U32:
      {
        uint32       scratchl[MAX_BIT_WIDTH>>5];
        uint32       scratchh[MAX_BIT_WIDTH>>5];
        uint32       end_mask = 0xffffffff >> (31 - ((vec->width - 1) & 0x1f));
        unsigned int i;
        for( i=0; i<(VECTOR_SIZE32(vec->width)-1); i++ ) {
          scratchl[i] = 0;
          scratchh[i] = 0xffffffff;
        }
        scratchl[i] = 0;
        scratchh[i] = end_mask;
        retval = vector_set_coverage_and_assign_uint32( vec, scratchl, scratchh, 0, (vec->width - 1) );
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

}

/*!
 \param vec  Pointer to vector to check for set bits

 \return Returns TRUE if the specified vector has been previously set (simulated); otherwise,
         returns FALSE.
*/
bool vector_is_set(
  const vector* vec
) { PROFILE(VECTOR_IS_SET);

  unsigned int i = 0;  /* Loop iterator */
  unsigned int size;   /* Size of data array */

  assert( vec != NULL );
  assert( vec->value.u32 != NULL );

  switch( vec->suppl.part.data_type ) {
    case VDATA_U32 :
      size = VECTOR_SIZE32( vec->width );
      while( (i < size) && (vec->value.u32[i][VTYPE_INDEX_SIG_SET] == 0) ) i++;
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( i < size );

}

/*!
 \param vec  Pointer to vector to convert into integer.

 \return Returns integer value of specified vector.

 Converts a vector structure into an integer value.  If the number of bits for the
 vector exceeds the number of bits in an integer, the upper bits of the vector are
 unused.
*/
int vector_to_int(
  const vector* vec
) { PROFILE(VECTOR_TO_INT);

  int retval;  /* Integer value returned to calling function */

  switch( vec->suppl.part.data_type ) {
    case VDATA_U32 :  retval = vec->value.u32[0][VTYPE_INDEX_VAL_VALL];  break;
    default        :  assert( 0 );  break;
  }

  /* If the vector is signed, sign-extend the integer */
  if( vec->suppl.part.is_signed == 1 ) {
    int width = (vec->width > 32) ? 32 : vec->width;
    retval |= (0xffffffff * ((retval >> (width - 1)) & 0x1)) << width;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \param vec  Pointer to vector to convert into integer.

 \return Returns integer value of specified vector.

 Converts a vector structure into an integer value.  If the number of bits for the
 vector exceeds the number of bits in an integer, the upper bits of the vector are
 unused.
*/
uint64 vector_to_uint64(
  const vector* vec
) { PROFILE(VECTOR_TO_UINT64);

  uint64 retval = 0;   /* 64-bit integer value returned to calling function */

  switch( vec->suppl.part.data_type ) {
    case VDATA_U32 :
      if( vec->width > 32 ) {
        retval = ((uint64)vec->value.u32[1][VTYPE_INDEX_VAL_VALL] << 32) | (uint64)vec->value.u32[0][VTYPE_INDEX_VAL_VALL];
      } else {
        retval = (uint64)vec->value.u32[0][VTYPE_INDEX_VAL_VALL];
      }
      break;
    default :  assert( 0 );  break;
  }

  /* If the vector is signed, sign-extend the integer */
  if( vec->suppl.part.is_signed == 1 ) {
    int width = (vec->width > 64) ? 64 : vec->width;
    retval |= (UINT64(0xffffffffffffffff) * ((retval >> (width - 1)) & 0x1)) << width;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \param vec   Pointer to vector to convert into integer.
 \param time  Pointer to sim_time structure to populate.

 Converts a vector structure into a sim_time structure.  If the number of bits for the
 vector exceeds the number of bits in an 64-bit integer, the upper bits of the vector are
 unused.
*/
void vector_to_sim_time(
  const vector* vec,
  sim_time*     time
) { PROFILE(VECTOR_TO_SIM_TIME);

  switch( vec->suppl.part.data_type ) {
    case VDATA_U32 :
      assert( (vec->value.u32[0][VTYPE_INDEX_VAL_VALH] == 0) && (vec->value.u32[1][VTYPE_INDEX_VAL_VALH] == 0) );
      time->lo   = vec->value.u32[0][VTYPE_INDEX_VAL_VALL];
      time->hi   = (vec->width > 32) ? vec->value.u32[1][VTYPE_INDEX_VAL_VALL] : 0;
      time->full = (((uint64)time->hi) << 32) | time->lo;
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

}

/*!
 \param vec    Pointer to vector store value into.
 \param value  Integer value to convert into vector.

 Converts an integer value into a vector, creating a vector value to store the
 new vector into.  This function is used along with the vector_to_int for
 mathematical vector operations.  We will first convert vectors into integers,
 perform the mathematical operation, and then revert the integers back into
 the vectors.
*/
void vector_from_int(
  vector* vec,
  int     value
) { PROFILE(VECTOR_FROM_INT);

  switch( vec->suppl.part.data_type ) {
    case VDATA_U32 :
      vec->value.u32[0][VTYPE_INDEX_VAL_VALL] = value & (0xffffffff >> (31 - ((vec->width - 1) & 0x1f)));
      break;
    default :  assert( 0 );  break;
  }

  /* Because this value came from an integer, specify that the vector is signed */
  vec->suppl.part.is_signed = 1;

  PROFILE_END;

}

/*!
 \param vec    Pointer to vector store value into.
 \param value  64-bit integer value to convert into vector.

 Converts a 64-bit integer value into a vector.  This function is used along with
 the vector_to_uint64 for mathematical vector operations.  We will first convert
 vectors into 64-bit integers, perform the mathematical operation, and then revert
 the 64-bit integers back into the vectors.
*/
void vector_from_uint64(
  vector* vec,
  uint64  value
) { PROFILE(VECTOR_FROM_UINT64);

  switch( vec->suppl.part.data_type ) {
    case VDATA_U32 :
      vec->value.u32[0][VTYPE_INDEX_VAL_VALL] = value & (0xffffffff >> (31 - ((vec->width - 1) & 0x1f)));
      vec->value.u32[1][VTYPE_INDEX_VAL_VALL] = ((uint64)value >> 32) & (0xffffffff >> (31 - ((vec->width - 1) & 0x1f)));
      break;
    default :  assert( 0 );  break;
  }

  /* Because this value came from an unsigned integer, specify that the vector is unsigned */
  vec->suppl.part.is_signed = 0;

  PROFILE_END;

}

/*!
 \param vec            Pointer to vector to add static value to.
 \param str            Value string to add.
 \param bits_per_char  Number of bits necessary to store a value character (1, 3, or 4).

 Iterates through string str starting at the left-most character, calculates the int value
 of the character and sets the appropriate number of bits in the specified vector locations.
*/
static void vector_set_static(
  vector* vec,
  char*   str,
  int     bits_per_char
) { PROFILE(VECTOR_SET_STATIC);

  char*        ptr       = str + (strlen( str ) - 1);  /* Pointer to current character evaluating */
  unsigned int pos       = 0;                          /* Current bit position in vector */
  unsigned int i;                                      /* Loop iterator */
  uint32       u32l      = 0;                          /* 32-bit unsigned integer */
  uint32       u32h      = 0;                          /* 32-bit unsigned integer */
  int          data_type = vec->suppl.part.data_type;  /* Copy of data type for performance reasons */

  while( ptr >= str ) {
    if( *ptr != '_' ) {
      if( (*ptr == 'x') || (*ptr == 'X') ) {
        switch( data_type ) {
          case VDATA_U32 :
            for( i=0; i<bits_per_char; i++ ) {
              if( (i + pos) < vec->width ) {
                vec->value.u32[(i+pos)>>5][VTYPE_INDEX_VAL_VALH] |= (1 << ((i+pos) & 0x1f));
              }
            }
            break;
          default :  assert( 0 );  break;
        }
      } else if( (*ptr == 'z') || (*ptr == 'Z') || (*ptr == '?') ) {
        switch( data_type ) {
          case VDATA_U32 :
            for( i=0; i<bits_per_char; i++ ) {
              if( (i + pos) < vec->width ) {
                unsigned int index = ((i + pos) >> 5);
                unsigned int value = (1 << ((i + pos) & 0x1f));
                vec->value.u32[index][VTYPE_INDEX_VAL_VALL] |= value;
                vec->value.u32[index][VTYPE_INDEX_VAL_VALH] |= value;
              }
            }
            break;
          default :  assert( 0 );  break;
        }
      } else {
        unsigned int val;
        if( (*ptr >= 'a') && (*ptr <= 'f') ) {
          val = (*ptr - 'a') + 10;
        } else if( (*ptr >= 'A') && (*ptr <= 'F') ) {
          val = (*ptr - 'A') + 10;
        } else {
          assert( (*ptr >= '0') && (*ptr <= '9') );
	  val = *ptr - '0';
        }
        switch( data_type ) {
          case VDATA_U32 :
            for( i=0; i<bits_per_char; i++ ) {
              if( (i + pos) < vec->width ) {
                vec->value.u32[(i+pos)>>5][VTYPE_INDEX_VAL_VALL] |= ((val >> i) & 0x1) << ((i + pos) & 0x1f);
              }
            }
            break;
          default :  assert( 0 );  break;
        }
      }
      pos = pos + bits_per_char;
    }
    ptr--;
  }

  PROFILE_END;

}

/*!
 \param vec   Pointer to vector to convert.
 \param base  Base type of vector value.

 \return Returns pointer to the allocated/coverted string.

 Converts a vector value into a string, allocating the memory for the string in this
 function and returning a pointer to that string.  The type specifies what type of
 value to change vector into.
*/
char* vector_to_string(
  vector* vec,
  int     base
) { PROFILE(VECTOR_TO_STRING);

  char* str = NULL;  /* Pointer to allocated string */

  if( base == QSTRING ) {

    int i, j;
    int vec_size  = ((vec->width & 0x7) == 0) ? ((vec->width >> 3) + 1) : ((vec->width >> 3) + 2);
    int pos       = 0;

    /* Allocate memory for string from the heap */
    str = (char*)malloc_safe( vec_size );

    switch( vec->suppl.part.data_type ) {
      case VDATA_U32 :
        {
          int offset = (vec->width >> 3) & 0x3;
          for( i=VECTOR_SIZE32(vec->width); i--; ) {
            uint32 val = vec->value.u32[i][VTYPE_INDEX_VAL_VALL]; 
            for( j=(offset - 1); j>=0; j-- ) {
              str[pos] = (val >> (j * 8)) & 0xff;
              pos++;
            }
            offset = 4;
          }
        }
        break;
      default :  assert( 0 );  break;
    }

    str[pos] = '\0';

  } else if( base == DECIMAL ) {

    char width_str[20];

    snprintf( width_str, 20, "%d", vector_to_int( vec ) );
    str = strdup_safe( width_str );
 
  } else {

    unsigned int rv;
    char*        tmp;
    int          str_size;
    unsigned int group;
    char         type_char;
    char         width_str[20];
    int          vec_size  = ((vec->width & 0x7) == 0) ? ((vec->width >> 3) + 1) : ((vec->width >> 3) + 2);
    int          pos       = 0;

    switch( base ) {
      case BINARY :  
        vec_size  = (vec->width + 1);
        group     = 1;
        type_char = 'b';
        break;
      case OCTAL :  
        vec_size  = ((vec->width % 3) == 0) ? ((vec->width / 3) + 1)
                                            : ((vec->width / 3) + 2);
        group     = 3;
        type_char = 'o';
        break;
      case HEXIDECIMAL :  
        vec_size  = ((vec->width % 4) == 0) ? ((vec->width / 4) + 1)
                                            : ((vec->width / 4) + 2);
        group     = 4;
        type_char = 'h';
        break;
      default          :  
        assert( (base == BINARY) || (base == OCTAL)  || (base == HEXIDECIMAL) );
        /*@-unreachable@*/
        break;
        /*@=unreachable@*/
    }

    tmp = (char*)malloc_safe( vec_size );

    switch( vec->suppl.part.data_type ) {
      case VDATA_U32 :
        {
          uint32 value = 0;
          int    i;
          for( i=(vec->width - 1); i>=0; i-- ) {
            uint32* entry = vec->value.u32[i>>5];
            if( ((entry[VTYPE_INDEX_VAL_VALH] >> (i & 0x1f)) & 0x1) == 1 ) {
              value = ((entry[VTYPE_INDEX_VAL_VALL] >> (i & 0x1f)) & 0x1) + 16;
            } else {
              value = (value < 16) ? ((1 << ((i & 0x1f) % group)) | value) : value;
            }
            assert( pos < vec_size );
            if( (i % group) == 0 ) {
              switch( value ) {
                case 0x0 :  if( (pos > 0) || (i == 0) ) { tmp[pos] = '0';  pos++; }  break;
                case 0x1 :  tmp[pos] = '1';  pos++;  break;
                case 0x2 :  tmp[pos] = '2';  pos++;  break;
                case 0x3 :  tmp[pos] = '3';  pos++;  break;
                case 0x4 :  tmp[pos] = '4';  pos++;  break;
                case 0x5 :  tmp[pos] = '5';  pos++;  break;
                case 0x6 :  tmp[pos] = '6';  pos++;  break;
                case 0x7 :  tmp[pos] = '7';  pos++;  break;
                case 0x8 :  tmp[pos] = '8';  pos++;  break;
                case 0x9 :  tmp[pos] = '9';  pos++;  break;
                case 0xa :  tmp[pos] = 'A';  pos++;  break;
                case 0xb :  tmp[pos] = 'B';  pos++;  break;
                case 0xc :  tmp[pos] = 'C';  pos++;  break;
                case 0xd :  tmp[pos] = 'D';  pos++;  break;
                case 0xe :  tmp[pos] = 'E';  pos++;  break;
                case 0xf :  tmp[pos] = 'F';  pos++;  break;
                case 16  :  tmp[pos] = 'X';  pos++;  break;
                case 17  :  tmp[pos] = 'Z';  pos++;  break;
                default  :  
                  /* Value in vector_to_string exceeds allowed limit */
                  assert( value <= 17 );
                  /*@-unreachable@*/
                  break;
                  /*@=unreachable@*/
              }
              value = 0;
            }
          }
        }
        break;
      default :  assert( 0 );  break;
    }

    tmp[pos] = '\0';

    rv = snprintf( width_str, 20, "%d", vec->width );
    assert( rv < 20 );
    str_size = strlen( width_str ) + 2 + strlen( tmp ) + 1 + vec->suppl.part.is_signed;
    str      = (char*)malloc_safe( str_size );
    if( vec->suppl.part.is_signed == 0 ) {
      rv = snprintf( str, str_size, "%d'%c%s", vec->width, type_char, tmp );
    } else {
      rv = snprintf( str, str_size, "%d's%c%s", vec->width, type_char, tmp );
    }
    assert( rv < str_size );

    free_safe( tmp, vec_size );

  }

  PROFILE_END;

  return( str );

}

/*!
 Converts a string value from the lexer into a vector structure appropriately
 sized.  If the string value size exceeds Covered's maximum bit allowance, return
 a value of NULL to indicate this to the calling function.
*/
void vector_from_string(
  char**   str,    /*!< String version of value */
  bool     quoted, /*!< If TRUE, treat the string as a literal */
  vector** vec,    /*!< Pointer to vector to allocate and populate with string information */
  int*     base    /*!< Base type of string value parsed */
) { PROFILE(VECTOR_FROM_STRING);

  int          bits_per_char;         /* Number of bits represented by a single character in the value string str */
  int          size;                  /* Specifies bit width of vector to create */
  char         value[MAX_BIT_WIDTH];  /* String to store string value in */
  char         stype[2];              /* Temporary holder for type of string being parsed */
  int          chars_read;            /* Number of characters read by a sscanf() function call */
  int          i;                     /* Loop iterator */
  unsigned int j;                     /* Loop iterator */
  int          pos;                   /* Bit position */

  if( quoted ) {

    size = strlen( *str ) * 8;

    /* If we have exceeded the maximum number of bits, return a value of NULL */
    if( size > MAX_BIT_WIDTH ) {

      *vec  = NULL;
      *base = 0;

    } else {

      /* Create vector */
      *vec  = vector_create( size, VTYPE_VAL, VDATA_U32, TRUE );
      *base = QSTRING;
      pos   = 0;

      for( i=(strlen( *str ) - 1); i>=0; i-- ) {
        (*vec)->value.u32[pos>>2][VTYPE_INDEX_VAL_VALL] |= (uint32)((*str)[i]) << ((pos & 0x3) << 3);
        pos++;
      }

    }

  } else {

    if( sscanf( *str, "%d'%[sSdD]%[0-9]%n", &size, stype, value, &chars_read ) == 3 ) {
      bits_per_char = 10;
      *base         = DECIMAL;
      *str          = *str + chars_read;
    } else if( sscanf( *str, "%d'%[sSbB]%[01xXzZ_\?]%n", &size, stype, value, &chars_read ) == 3 ) {
      bits_per_char = 1;
      *base         = BINARY;
      *str          = *str + chars_read;
    } else if( sscanf( *str, "%d'%[sSoO]%[0-7xXzZ_\?]%n", &size, stype, value, &chars_read ) == 3 ) {
      bits_per_char = 3;
      *base         = OCTAL;
      *str          = *str + chars_read;
    } else if( sscanf( *str, "%d'%[sShH]%[0-9a-fA-FxXzZ_\?]%n", &size, stype, value, &chars_read ) == 3 ) {
      bits_per_char = 4;
      *base         = HEXIDECIMAL;
      *str          = *str + chars_read;
    } else if( sscanf( *str, "'%[sSdD]%[0-9]%n", stype, value, &chars_read ) == 2 ) {
      bits_per_char = 10;
      *base         = DECIMAL;
      size          = 32;
      *str          = *str + chars_read;
    } else if( sscanf( *str, "'%[sSbB]%[01xXzZ_\?]%n", stype, value, &chars_read ) == 2 ) {
      bits_per_char = 1;
      *base         = BINARY;
      size          = 32;
      *str          = *str + chars_read;
    } else if( sscanf( *str, "'%[sSoO]%[0-7xXzZ_\?]%n", stype, value, &chars_read ) == 2 ) {
      bits_per_char = 3;
      *base         = OCTAL;
      size          = 32;
      *str          = *str + chars_read;
    } else if( sscanf( *str, "'%[sShH]%[0-9a-fA-FxXzZ_\?]%n", stype, value, &chars_read ) == 2 ) {
      bits_per_char = 4;
      *base         = HEXIDECIMAL;
      size          = 32;
      *str          = *str + chars_read;
    } else if( sscanf( *str, "%[0-9_]%n", value, &chars_read ) == 1 ) {
      bits_per_char = 10;
      *base         = DECIMAL;
      stype[0]      = 's';       
      stype[1]      = '\0';
      size          = 32;
      *str          = *str + chars_read;
    } else {
      /* If the specified string is none of the above, return NULL */
      bits_per_char = 0;
    }

    /* If we have exceeded the maximum number of bits, return a value of NULL */
    if( (size > MAX_BIT_WIDTH) || (bits_per_char == 0) ) {

      *vec  = NULL;
      *base = 0;

    } else {

      /* Create vector */
      *vec = vector_create( size, VTYPE_VAL, VDATA_U32, TRUE );
      if( *base == DECIMAL ) {
        vector_from_int( *vec, ato32( value ) );
      } else {
        vector_set_static( *vec, value, bits_per_char ); 
      }

      /* Set the signed bit to the appropriate value based on the signed indicator in the vector string */
      if( (stype[0] == 's') || (stype [0] == 'S') ) {
        (*vec)->suppl.part.is_signed = 1;
      } else {
        (*vec)->suppl.part.is_signed = 0;
      }

    }

  }

  PROFILE_END;

}

/*!
 \param vec    Pointer to vector to set value to.
 \param value  String version of VCD value.
 \param msb    Most significant bit to assign to.
 \param lsb    Least significant bit to assign to.

 \return Returns TRUE if assigned value differs from the original value; otherwise,
         returns FALSE.

 \throws anonymous Throw

 Iterates through specified value string, setting the specified vector value to
 this value.  Performs a VCD-specific bit-fill if the value size is not the size
 of the vector.  The specified value string is assumed to be in binary format.
*/
bool vector_vcd_assign(
  vector*     vec,
  const char* value,
  int         msb,
  int         lsb
) { PROFILE(VECTOR_VCD_ASSIGN);

  bool          retval = FALSE;  /* Return value for this function */
  const char*   ptr;             /* Pointer to current character under evaluation */
  int           i;               /* Loop iterator */

  assert( vec != NULL );
  assert( value != NULL );
  assert( msb <= vec->width );

  /* Set pointer to LSB */
  ptr = (value + strlen( value )) - 1;
  i   = (lsb > 0) ? lsb : 0;
  msb = (lsb > 0) ? msb : msb;

  switch( vec->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        uint32 scratchl[MAX_BIT_WIDTH>>5];
        uint32 scratchh[MAX_BIT_WIDTH>>5];
        while( ptr >= value ) {
          unsigned int index  = (i >> 5);
          unsigned int offset = (i & 0x1f);
          uint32       bit    = (1 << offset);
          if( offset == 0 ) {
            scratchl[index] = 0;
            scratchh[index] = 0;
          }
          scratchl[index] |= ((*ptr == '1') || (*ptr == 'z')) ? bit : 0;
          scratchh[index] |= ((*ptr == 'x') || (*ptr == 'z')) ? bit : 0;
          ptr--;
          i++;
        }
        ptr++;
        /* Bit-fill */
        for( ; i<=msb; i++ ) {
          unsigned int index  = (i >> 5);
          unsigned int offset = (i & 0x1f);
          uint32       bit    = (1 << offset);
          if( offset == 0 ) {
            scratchl[index] = 0;
            scratchh[index] = 0;
          }
          scratchl[index] |= (*ptr == 'z') ? bit : 0;
          scratchh[index] |= ((*ptr == 'x') || (*ptr == 'z')) ? bit : 0;
        }
        retval = vector_set_coverage_and_assign_uint32( vec, scratchl, scratchh, lsb, msb );
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt    Target vector for operation results to be stored.
 \param src1   Source vector 1 to perform operation on.
 \param src2   Source vector 2 to perform operation on.

 \return Returns TRUE if assigned value differs from original vector value; otherwise,
         returns FALSE.

 Performs a bitwise AND operation.  Vector sizes will be properly compensated by
 placing zeroes.
*/
bool vector_bitwise_and_op(
  vector* tgt,
  vector* src1,
  vector* src2
) { PROFILE(VECTOR_BITWISE_AND_OP);
  
  bool retval;  /* Return value for this function */

  switch( tgt->suppl.part.data_type ) {
    case VDATA_U32 :
      { 
        static uint32 scratchl[MAX_BIT_WIDTH>>5];
        static uint32 scratchh[MAX_BIT_WIDTH>>5];
        int           src1_size = VECTOR_SIZE32(src1->width);
        int           src2_size = VECTOR_SIZE32(src2->width);
        unsigned int  i;
        for( i=0; i<VECTOR_SIZE32(tgt->width); i++ ) {
          uint32* entry1 = src1->value.u32[i];
          uint32* entry2 = src2->value.u32[i];
          uint32  val1_l = (i<src1_size) ? entry1[VTYPE_INDEX_VAL_VALL] : 0;
          uint32  val1_h = (i<src1_size) ? entry1[VTYPE_INDEX_VAL_VALH] : 0;
          uint32  val2_l = (i<src2_size) ? entry2[VTYPE_INDEX_VAL_VALL] : 0;
          uint32  val2_h = (i<src2_size) ? entry2[VTYPE_INDEX_VAL_VALH] : 0;
          scratchl[i]   = ~(val1_h | val2_h) & (val1_l & val2_l);
          scratchh[i]   = (val1_h & val2_h) | (val1_h & val2_l) | (val2_h & val1_l);
        }
        retval = vector_set_coverage_and_assign_uint32( tgt, scratchl, scratchh, 0, (tgt->width - 1) );
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );
 
}

/*!
 \param tgt    Target vector for operation results to be stored.
 \param src1   Source vector 1 to perform operation on.
 \param src2   Source vector 2 to perform operation on.

 \return Returns TRUE if assigned value differs from original vector value; otherwise,
         returns FALSE.

 Performs a bitwise NAND operation.  Vector sizes will be properly compensated by
 placing zeroes.
*/
bool vector_bitwise_nand_op(           
  vector* tgt,                        
  vector* src1,                       
  vector* src2                        
) { PROFILE(VECTOR_BITWISE_NAND_OP);   
                                      
  bool retval;  /* Return value for this function */
                                      
  switch( tgt->suppl.part.data_type ) {
    case VDATA_U32 :                  
      {
        static uint32 scratchl[MAX_BIT_WIDTH>>5];
        static uint32 scratchh[MAX_BIT_WIDTH>>5];
        int           src1_size = VECTOR_SIZE32(src1->width);
        int           src2_size = VECTOR_SIZE32(src2->width);
        unsigned int  i;
        for( i=0; i<VECTOR_SIZE32(tgt->width); i++ ) {
          uint32* entry1 = src1->value.u32[i];
          uint32* entry2 = src2->value.u32[i];
          uint32  val1_l = (i<src1_size) ? entry1[VTYPE_INDEX_VAL_VALL] : 0;
          uint32  val1_h = (i<src1_size) ? entry1[VTYPE_INDEX_VAL_VALH] : 0;
          uint32  val2_l = (i<src2_size) ? entry2[VTYPE_INDEX_VAL_VALL] : 0;
          uint32  val2_h = (i<src2_size) ? entry2[VTYPE_INDEX_VAL_VALH] : 0;
          scratchl[i] = ~(val1_h | val2_h) & ~(val1_l & val2_l);
          scratchh[i] = (val1_h & val2_h) | (val1_h & ~val2_l) | (val2_h & ~val1_l);
        }
        retval = vector_set_coverage_and_assign_uint32( tgt, scratchl, scratchh, 0, (tgt->width - 1) );
      }
      break;
    default :  assert( 0 );  break; 
  }

  PROFILE_END;

  return( retval );
 
}

/*!
 \param tgt    Target vector for operation results to be stored.
 \param src1   Source vector 1 to perform operation on.
 \param src2   Source vector 2 to perform operation on.

 \return Returns TRUE if assigned value differs from original vector value; otherwise,
         returns FALSE.

 Performs a bitwise OR operation.  Vector sizes will be properly compensated by
 placing zeroes.
*/
bool vector_bitwise_or_op(
  vector* tgt,
  vector* src1,
  vector* src2
) { PROFILE(VECTOR_BITWISE_OR_OP);

  bool retval;  /* Return value for this function */

  switch( tgt->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        static uint32 scratchl[MAX_BIT_WIDTH>>5];
        static uint32 scratchh[MAX_BIT_WIDTH>>5];
        int           src1_size = VECTOR_SIZE32(src1->width);
        int           src2_size = VECTOR_SIZE32(src2->width);
        unsigned int  i;
        for( i=0; i<VECTOR_SIZE32(tgt->width); i++ ) {
          uint32* entry1 = src1->value.u32[i];
          uint32* entry2 = src2->value.u32[i];
          uint32  val1_l = (i<src1_size) ? entry1[VTYPE_INDEX_VAL_VALL] : 0;
          uint32  val1_h = (i<src1_size) ? entry1[VTYPE_INDEX_VAL_VALH] : 0;
          uint32  val2_l = (i<src2_size) ? entry2[VTYPE_INDEX_VAL_VALL] : 0;
          uint32  val2_h = (i<src2_size) ? entry2[VTYPE_INDEX_VAL_VALH] : 0;
          scratchl[i] = ~(val1_h | val2_h) & (val1_l |  val2_l);
          scratchh[i] =  (val1_h & val2_h) | (val1_h & ~val2_l) | (val2_h & ~val1_l);
        }
        retval = vector_set_coverage_and_assign_uint32( tgt, scratchl, scratchh, 0, (tgt->width - 1) );
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt    Target vector for operation results to be stored.
 \param src1   Source vector 1 to perform operation on.
 \param src2   Source vector 2 to perform operation on.

 \return Returns TRUE if assigned value differs from original vector value; otherwise,
         returns FALSE.

 Performs a bitwise NOR operation.  Vector sizes will be properly compensated by
 placing zeroes.
*/
bool vector_bitwise_nor_op(
  vector* tgt,
  vector* src1,
  vector* src2
) { PROFILE(VECTOR_BITWISE_NOR_OP);

  bool retval;  /* Return value for this function */

  switch( tgt->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        static uint32 scratchl[MAX_BIT_WIDTH>>5];
        static uint32 scratchh[MAX_BIT_WIDTH>>5];
        int           src1_size = VECTOR_SIZE32(src1->width);
        int           src2_size = VECTOR_SIZE32(src2->width);
        unsigned int  i;
        for( i=0; i<VECTOR_SIZE32(tgt->width); i++ ) {
          uint32* entry1 = src1->value.u32[i];
          uint32* entry2 = src2->value.u32[i];
          uint32  val1_l = (i<src1_size) ? entry1[VTYPE_INDEX_VAL_VALL] : 0;
          uint32  val1_h = (i<src1_size) ? entry1[VTYPE_INDEX_VAL_VALH] : 0;
          uint32  val2_l = (i<src2_size) ? entry2[VTYPE_INDEX_VAL_VALL] : 0;
          uint32  val2_h = (i<src2_size) ? entry2[VTYPE_INDEX_VAL_VALH] : 0;
          scratchl[i] = ~(val1_h | val2_h) & ~(val1_l | val2_l);
          scratchh[i] =  (val1_h & val2_h) |  (val1_h & val2_l) | (val2_h & val1_l);
        }
        retval = vector_set_coverage_and_assign_uint32( tgt, scratchl, scratchh, 0, (tgt->width - 1) );
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt    Target vector for operation results to be stored.
 \param src1   Source vector 1 to perform operation on.
 \param src2   Source vector 2 to perform operation on.

 \return Returns TRUE if assigned value differs from original vector value; otherwise,
         returns FALSE.

 Performs a bitwise XOR operation.  Vector sizes will be properly compensated by
 placing zeroes.
*/
bool vector_bitwise_xor_op(
  vector* tgt,
  vector* src1,
  vector* src2
) { PROFILE(VECTOR_BITWISE_XOR_OP);

  bool retval;  /* Return value for this function */

  switch( tgt->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        static uint32 scratchl[MAX_BIT_WIDTH>>5];
        static uint32 scratchh[MAX_BIT_WIDTH>>5];
        int           src1_size = VECTOR_SIZE32(src1->width);
        int           src2_size = VECTOR_SIZE32(src2->width);
        unsigned int  i;
        for( i=0; i<VECTOR_SIZE32(tgt->width); i++ ) {
          uint32* entry1 = src1->value.u32[i];
          uint32* entry2 = src2->value.u32[i];
          uint32  val1_l = (i<src1_size) ? entry1[VTYPE_INDEX_VAL_VALL] : 0;
          uint32  val1_h = (i<src1_size) ? entry1[VTYPE_INDEX_VAL_VALH] : 0;
          uint32  val2_l = (i<src2_size) ? entry2[VTYPE_INDEX_VAL_VALL] : 0;
          uint32  val2_h = (i<src2_size) ? entry2[VTYPE_INDEX_VAL_VALH] : 0;
          scratchl[i] = (val1_l ^ val2_l) & ~(val1_h | val2_h);
          scratchh[i] = (val1_h | val2_h);
        }
        retval = vector_set_coverage_and_assign_uint32( tgt, scratchl, scratchh, 0, (tgt->width - 1) );
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt    Target vector for operation results to be stored.
 \param src1   Source vector 1 to perform operation on.
 \param src2   Source vector 2 to perform operation on.

 \return Returns TRUE if assigned value differs from original vector value; otherwise,
         returns FALSE.

 Performs a bitwise NXOR operation.  Vector sizes will be properly compensated by
 placing zeroes.
*/
bool vector_bitwise_nxor_op(
  vector* tgt,
  vector* src1,
  vector* src2
) { PROFILE(VECTOR_BITWISE_NXOR_OP);
  
  bool retval;  /* Return value for this function */

  switch( tgt->suppl.part.data_type ) {
    case VDATA_U32 :
      { 
        static uint32 scratchl[MAX_BIT_WIDTH>>5];
        static uint32 scratchh[MAX_BIT_WIDTH>>5];
        int           src1_size = VECTOR_SIZE32(src1->width);
        int           src2_size = VECTOR_SIZE32(src2->width);
        unsigned int  i;
        for( i=0; i<VECTOR_SIZE32(tgt->width); i++ ) {
          uint32* entry1 = src1->value.u32[i];
          uint32* entry2 = src2->value.u32[i];
          uint32  val1_l = (i<src1_size) ? entry1[VTYPE_INDEX_VAL_VALL] : 0;
          uint32  val1_h = (i<src1_size) ? entry1[VTYPE_INDEX_VAL_VALH] : 0;
          uint32  val2_l = (i<src2_size) ? entry2[VTYPE_INDEX_VAL_VALL] : 0;
          uint32  val2_h = (i<src2_size) ? entry2[VTYPE_INDEX_VAL_VALH] : 0;
          scratchl[i] = ~(val1_l ^ val2_l) & ~(val1_h | val2_h);
          scratchh[i] =  (val1_h | val2_h);
        }
        retval = vector_set_coverage_and_assign_uint32( tgt, scratchl, scratchh, 0, (tgt->width - 1) );
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );
 
}

/*!
 \param tgt    Target vector for storage of results.
 \param left   Expression on left of less than sign.
 \param right  Expression on right of less than sign.

 \return Returns TRUE if the assigned value differs from the original value; otherwise, returns FALSE.

 Performs a less-than comparison of the left and right expressions.
*/
bool vector_op_lt(
  vector* tgt,
  vector* left,
  vector* right
) { PROFILE(VECTOR_OP_LT);

  bool retval;  /* Return value for this function */

  switch( tgt->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        uint32       scratchl = 0;
        uint32       scratchh = 0;
        unsigned int lsize = VECTOR_SIZE32(left->width);
        unsigned int rsize = VECTOR_SIZE32(right->width);
        int          i     = ((lsize < rsize) ? rsize : lsize);
        uint32       lvall;
        uint32       lvalh;
        uint32       rvall;
        uint32       rvalh;
        do {
          i--;
          lvall = (i<lsize) ? left->value.u32[i][VTYPE_INDEX_VAL_VALL]  : 0;
          lvalh = (i<lsize) ? left->value.u32[i][VTYPE_INDEX_VAL_VALH]  : 0xffffffff;
          rvall = (i<rsize) ? right->value.u32[i][VTYPE_INDEX_VAL_VALL] : 0;
          rvalh = (i<rsize) ? right->value.u32[i][VTYPE_INDEX_VAL_VALH] : 0xffffffff;
          printf( "i: %d, lvall: %x, rvall: %x, lvalh: %x, rvalh: %x\n", i, lvall, rvall, lvalh, rvalh );
        } while( (i >= 0) && (lvall == rvall) && (lvalh == 0) && (rvalh == 0) );
        if( (lvalh != 0) || (rvalh != 0) ) {
          scratchh = 1;
        } else {
          scratchl = (lvall < rvall);
        }
        printf( "scratchl: %d, scratchh: %d\n", scratchl, scratchh );
        retval = vector_set_coverage_and_assign_uint32( tgt, &scratchl, &scratchh, 0, 0 );
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt    Target vector for storage of results.
 \param left   Expression on left of less than sign.
 \param right  Expression on right of less than sign.

 \return Returns TRUE if the assigned value differs from the original value; otherwise, returns FALSE.

 Performs a less-than-or-equal comparison of the left and right expressions.
*/
bool vector_op_le(
  vector* tgt,
  vector* left,
  vector* right
) { PROFILE(VECTOR_OP_LE);

  bool retval;  /* Return value for this function */

  switch( tgt->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        uint32       scratchl = 0;
        uint32       scratchh = 0;
        unsigned int lsize = VECTOR_SIZE32(left->width);
        unsigned int rsize = VECTOR_SIZE32(right->width);
        int          i     = ((lsize < rsize) ? rsize : lsize);
        uint32       lvall;
        uint32       lvalh;
        uint32       rvall;
        uint32       rvalh;
        do {
          i--;
          lvall = (i<lsize) ? left->value.u32[i][VTYPE_INDEX_VAL_VALL]  : 0;
          lvalh = (i<lsize) ? left->value.u32[i][VTYPE_INDEX_VAL_VALH]  : 0xffffffff;
          rvall = (i<rsize) ? right->value.u32[i][VTYPE_INDEX_VAL_VALL] : 0;
          rvalh = (i<rsize) ? right->value.u32[i][VTYPE_INDEX_VAL_VALH] : 0xffffffff;
        } while( (i >= 0) && (lvall == rvall) && (lvalh == 0) && (rvalh == 0) );
        if( (lvalh != 0) || (rvalh != 0) ) {
          scratchh = 1;
        } else {
          scratchl = (lvall <= rvall);
        }
        retval = vector_set_coverage_and_assign_uint32( tgt, &scratchl, &scratchh, 0, 0 );
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt    Target vector for storage of results.
 \param left   Expression on left of greater-than sign.
 \param right  Expression on right of greater-than sign.

 \return Returns TRUE if the assigned value differs from the original value; otherwise, returns FALSE.

 Performs a greater-than comparison of the left and right expressions.
*/
bool vector_op_gt(
  vector* tgt,
  vector* left,
  vector* right
) { PROFILE(VECTOR_OP_GT);

  bool retval;  /* Return value for this function */

  switch( tgt->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        uint32       scratchl = 0;
        uint32       scratchh = 0;
        unsigned int lsize = VECTOR_SIZE32(left->width);
        unsigned int rsize = VECTOR_SIZE32(right->width);
        int          i     = ((lsize < rsize) ? rsize : lsize);
        uint32       lvall;
        uint32       lvalh;
        uint32       rvall;
        uint32       rvalh;
        do {
          i--;
          lvall = (i<lsize) ? left->value.u32[i][VTYPE_INDEX_VAL_VALL]  : 0;
          lvalh = (i<lsize) ? left->value.u32[i][VTYPE_INDEX_VAL_VALH]  : 0xffffffff;
          rvall = (i<rsize) ? right->value.u32[i][VTYPE_INDEX_VAL_VALL] : 0;
          rvalh = (i<rsize) ? right->value.u32[i][VTYPE_INDEX_VAL_VALH] : 0xffffffff;
        } while( (i >= 0) && (lvall == rvall) && (lvalh == 0) && (rvalh == 0) );
        if( (lvalh != 0) || (rvalh != 0) ) {
          scratchh = 1;
        } else {
          scratchl = (lvall > rvall);
        }
        retval = vector_set_coverage_and_assign_uint32( tgt, &scratchl, &scratchh, 0, 0 );
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt    Target vector for storage of results.
 \param left   Expression on left of greater-than-or-equal sign.
 \param right  Expression on right of greater-than-or-equal sign.

 \return Returns TRUE if the assigned value differs from the original value; otherwise, returns FALSE.

 Performs a greater-than-or-equal comparison of the left and right expressions.
*/
bool vector_op_ge(
  vector* tgt,
  vector* left,
  vector* right
) { PROFILE(VECTOR_OP_GE);

  bool retval;  /* Return value for this function */

  switch( tgt->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        uint32       scratchl = 0;
        uint32       scratchh = 0;
        unsigned int lsize = VECTOR_SIZE32(left->width);
        unsigned int rsize = VECTOR_SIZE32(right->width);
        int          i     = ((lsize < rsize) ? rsize : lsize);
        uint32       lvall;
        uint32       lvalh;
        uint32       rvall;
        uint32       rvalh;
        do {
          i--;
          lvall = (i<lsize) ? left->value.u32[i][VTYPE_INDEX_VAL_VALL]  : 0;
          lvalh = (i<lsize) ? left->value.u32[i][VTYPE_INDEX_VAL_VALH]  : 0xffffffff;
          rvall = (i<rsize) ? right->value.u32[i][VTYPE_INDEX_VAL_VALL] : 0;
          rvalh = (i<rsize) ? right->value.u32[i][VTYPE_INDEX_VAL_VALH] : 0xffffffff;
        } while( (i >= 0) && (lvall == rvall) && (lvalh == 0) && (rvalh == 0) );
        if( (lvalh != 0) || (rvalh != 0) ) {
          scratchh = 1;
        } else {
          scratchl = (lvall >= rvall);
        }
        retval = vector_set_coverage_and_assign_uint32( tgt, &scratchl, &scratchh, 0, 0 );
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt    Target vector for storage of results.
 \param left   Expression on left of greater-than-or-equal sign.
 \param right  Expression on right of greater-than-or-equal sign.

 \return Returns TRUE if the assigned value differs from the original value; otherwise, returns FALSE.

 Performs an equal comparison of the left and right expressions.
*/
bool vector_op_eq(
  vector* tgt,
  vector* left,
  vector* right
) { PROFILE(VECTOR_OP_EQ);

  bool retval;  /* Return value for this function */

  switch( tgt->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        uint32       scratchl = 0;
        uint32       scratchh = 0;
        unsigned int lsize = VECTOR_SIZE32(left->width);
        unsigned int rsize = VECTOR_SIZE32(right->width);
        int          i     = ((lsize < rsize) ? rsize : lsize);
        uint32       lvall;
        uint32       lvalh;
        uint32       rvall;
        uint32       rvalh;
        do {
          i--;
          lvall = (i<lsize) ? left->value.u32[i][VTYPE_INDEX_VAL_VALL]  : 0;
          lvalh = (i<lsize) ? left->value.u32[i][VTYPE_INDEX_VAL_VALH]  : 0xffffffff;
          rvall = (i<rsize) ? right->value.u32[i][VTYPE_INDEX_VAL_VALL] : 0;
          rvalh = (i<rsize) ? right->value.u32[i][VTYPE_INDEX_VAL_VALH] : 0xffffffff;
        } while( (i >= 0) && (lvall == rvall) && (lvalh == 0) && (rvalh == 0) );
        if( (lvalh != 0) || (rvalh != 0) ) {
          scratchh = 1;
        } else {
          scratchl = (lvall == rvall);
        }
        retval = vector_set_coverage_and_assign_uint32( tgt, &scratchl, &scratchh, 0, 0 );
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt    Target vector for storage of results.
 \param left   Expression on left of greater-than-or-equal sign.
 \param right  Expression on right of greater-than-or-equal sign.

 \return Returns TRUE if the assigned value differs from the original value; otherwise, returns FALSE.

 Performs a case equal comparison of the left and right expressions.
*/
bool vector_op_ceq(
  vector* tgt,
  vector* left,
  vector* right
) { PROFILE(VECTOR_OP_CEQ);

  bool retval;  /* Return value for this function */

  switch( tgt->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        uint32       scratchl = 0;
        uint32       scratchh = 0;
        unsigned int lsize = VECTOR_SIZE32(left->width);
        unsigned int rsize = VECTOR_SIZE32(right->width);
        int          i     = ((lsize < rsize) ? rsize : lsize);
        uint32       lvall;
        uint32       lvalh;
        uint32       rvall;
        uint32       rvalh;
        do {
          i--;
          lvall = (i<lsize) ? left->value.u32[i][VTYPE_INDEX_VAL_VALL]  : 0;
          lvalh = (i<lsize) ? left->value.u32[i][VTYPE_INDEX_VAL_VALH]  : 0xffffffff;
          rvall = (i<rsize) ? right->value.u32[i][VTYPE_INDEX_VAL_VALL] : 0;
          rvalh = (i<rsize) ? right->value.u32[i][VTYPE_INDEX_VAL_VALH] : 0xffffffff;
        } while( (i >= 0) && ((lvall & ~lvalh) == (rvall & ~rvalh)) );
        scratchl = (lvall == rvall);
        retval   = vector_set_coverage_and_assign_uint32( tgt, &scratchl, &scratchh, 0, 0 );
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt    Target vector for storage of results.
 \param left   Expression on left of greater-than-or-equal sign.
 \param right  Expression on right of greater-than-or-equal sign.

 \return Returns TRUE if the assigned value differs from the original value; otherwise, returns FALSE.

 Performs a casex equal comparison of the left and right expressions.
*/
bool vector_op_cxeq(
  vector* tgt,
  vector* left,
  vector* right
) { PROFILE(VECTOR_OP_CXEQ);

  bool retval;  /* Return value for this function */

  switch( tgt->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        uint32       scratchl = 0;
        uint32       scratchh = 0;
        unsigned int lsize = VECTOR_SIZE32(left->width);
        unsigned int rsize = VECTOR_SIZE32(right->width);
        int          i     = ((lsize < rsize) ? rsize : lsize);
        uint32       lvall;
        uint32       lvalh;
        uint32       rvall;
        uint32       rvalh;
        do {
          i--;
          lvall = (i<lsize) ? left->value.u32[i][VTYPE_INDEX_VAL_VALL]  : 0;
          lvalh = (i<lsize) ? left->value.u32[i][VTYPE_INDEX_VAL_VALH]  : 0xffffffff;
          rvall = (i<rsize) ? right->value.u32[i][VTYPE_INDEX_VAL_VALL] : 0;
          rvalh = (i<rsize) ? right->value.u32[i][VTYPE_INDEX_VAL_VALH] : 0xffffffff;
        } while( (i >= 0) && ((lvall == rvall) || (lvalh == 0) || (rvall == 0)) );
        scratchl = (lvall == rvall);
        retval   = vector_set_coverage_and_assign_uint32( tgt, &scratchl, &scratchh, 0, 0 );
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt    Target vector for storage of results.
 \param left   Expression on left of greater-than-or-equal sign.
 \param right  Expression on right of greater-than-or-equal sign.

 \return Returns TRUE if the assigned value differs from the original value; otherwise, returns FALSE.

 Performs a casez equal comparison of the left and right expressions.
*/
bool vector_op_czeq(
  vector* tgt,
  vector* left,
  vector* right
) { PROFILE(VECTOR_OP_CZEQ);

  bool retval;  /* Return value for this function */

  switch( tgt->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        uint32       scratchl = 0;
        uint32       scratchh = 0;
        unsigned int lsize = VECTOR_SIZE32(left->width);
        unsigned int rsize = VECTOR_SIZE32(right->width);
        int          i     = ((lsize < rsize) ? rsize : lsize);
        uint32       lvall;
        uint32       lvalh;
        uint32       rvall;
        uint32       rvalh;
        do {
          i--;
          lvall = (i<lsize) ? left->value.u32[i][VTYPE_INDEX_VAL_VALL]  : 0;
          lvalh = (i<lsize) ? left->value.u32[i][VTYPE_INDEX_VAL_VALH]  : 0xffffffff;
          rvall = (i<rsize) ? right->value.u32[i][VTYPE_INDEX_VAL_VALL] : 0;
          rvalh = (i<rsize) ? right->value.u32[i][VTYPE_INDEX_VAL_VALH] : 0xffffffff;
        } while( (i >= 0) && ((lvall == rvall) || ~(lvalh & lvall) || ~(rvalh & rvall)) );
        scratchl = (lvall == rvall);
        retval   = vector_set_coverage_and_assign_uint32( tgt, &scratchl, &scratchh, 0, 0 );
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt    Target vector for storage of results.
 \param left   Expression on left of greater-than-or-equal sign.
 \param right  Expression on right of greater-than-or-equal sign.

 \return Returns TRUE if the assigned value differs from the original value; otherwise, returns FALSE.

 Performs a not-equal comparison of the left and right expressions.
*/
bool vector_op_ne(
  vector* tgt,
  vector* left,
  vector* right
) { PROFILE(VECTOR_OP_NE);

  bool retval;  /* Return value for this function */

  switch( tgt->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        uint32       scratchl = 0;
        uint32       scratchh = 0;
        unsigned int lsize = VECTOR_SIZE32(left->width);
        unsigned int rsize = VECTOR_SIZE32(right->width);
        int          i     = ((lsize < rsize) ? rsize : lsize);
        uint32       lvall;
        uint32       lvalh;
        uint32       rvall;
        uint32       rvalh;
        do {
          i--;
          lvall = (i<lsize) ? left->value.u32[i][VTYPE_INDEX_VAL_VALL]  : 0;
          lvalh = (i<lsize) ? left->value.u32[i][VTYPE_INDEX_VAL_VALH]  : 0;
          rvall = (i<rsize) ? right->value.u32[i][VTYPE_INDEX_VAL_VALL] : 0;
          rvalh = (i<rsize) ? right->value.u32[i][VTYPE_INDEX_VAL_VALH] : 0;
        } while( (i >= 0) && (lvall == rvall) && (lvalh == 0) && (rvalh == 0) );
        if( (lvalh != 0) || (rvalh != 0) ) {
          scratchh = 1;
        } else {
          scratchl = (lvall != rvall);
        }
        retval = vector_set_coverage_and_assign_uint32( tgt, &scratchl, &scratchh, 0, 0 );
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt    Target vector for storage of results.
 \param left   Expression on left of greater-than-or-equal sign.
 \param right  Expression on right of greater-than-or-equal sign.
 
 \return Returns TRUE if the assigned value differs from the original value; otherwise, returns FALSE.
 
 Performs an case not-equal comparison of the left and right expressions.
*/
bool vector_op_cne(
  vector* tgt,
  vector* left,
  vector* right
) { PROFILE(VECTOR_OP_CNE);

  bool retval;  /* Return value for this function */
  
  switch( tgt->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        uint32       scratchl = 0;
        uint32       scratchh = 0;
        unsigned int lsize = VECTOR_SIZE32(left->width);
        unsigned int rsize = VECTOR_SIZE32(right->width);
        int          i     = ((lsize < rsize) ? rsize : lsize);
        uint32       lvall;
        uint32       lvalh;
        uint32       rvall;
        uint32       rvalh;
        do {
          i--;
          lvall = (i<lsize) ? left->value.u32[i][VTYPE_INDEX_VAL_VALL]  : 0;
          lvalh = (i<lsize) ? left->value.u32[i][VTYPE_INDEX_VAL_VALH]  : 0xffffffff;
          rvall = (i<rsize) ? right->value.u32[i][VTYPE_INDEX_VAL_VALL] : 0;
          rvalh = (i<rsize) ? right->value.u32[i][VTYPE_INDEX_VAL_VALH] : 0xffffffff;
        } while( (i >= 0) && ((lvall & ~lvalh) == (rvall & ~rvalh)) );
        scratchl = (lvall != rvall); 
        retval   = vector_set_coverage_and_assign_uint32( tgt, &scratchl, &scratchh, 0, 0 );
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \return Returns TRUE if the value of tgt changed as a result of this operation; otherwise, returns FALSE.

 Performs logical-OR operation and calculates coverage information.
*/
bool vector_op_lor(
  vector* tgt,   /*!< Pointer to vector to store result into */
  vector* left,  /*!< Pointer to left vector of expression */
  vector* right  /*!< Pointer to right vector of expression */
) { PROFILE(VECTOR_OP_LOR);

  bool retval;                                 /* Return value for this function */
  bool lunknown = vector_is_unknown( left );   /* Check for left value being unknown */
  bool runknown = vector_is_unknown( right );  /* Check for right value being unknown */

  switch( tgt->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        uint32 valh = (lunknown && runknown) ? 1 : 0;
        uint32 vall = ((!lunknown && vector_is_not_zero( left )) || (!runknown && vector_is_not_zero( right ))) ? 1 : 0;
        retval      = vector_set_coverage_and_assign_uint32( tgt, &vall, &valh, 0, 0 );
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \return Returns TRUE if the value of tgt changed as a result of this operation; otherwise, returns FALSE.

 Performs logical-AND operation and calculates coverage information.
*/
bool vector_op_land(
  vector* tgt,   /*!< Pointer to vector to store result into */
  vector* left,  /*!< Pointer to left vector of expression */
  vector* right  /*!< Pointer to right vector of expression */
) { PROFILE(VECTOR_OP_LAND);

  bool retval;                                 /* Return value for this function */
  bool lunknown = vector_is_unknown( left );   /* Check for left value being unknown */
  bool runknown = vector_is_unknown( right );  /* Check for right value being unknown */

  switch( tgt->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        uint32 valh = (lunknown && runknown) ? 1 : 0;
        uint32 vall = ((!lunknown && !vector_is_not_zero( left )) || (!runknown && !vector_is_not_zero( right ))) ? 1 : 0;
        retval      = vector_set_coverage_and_assign_uint32( tgt, &vall, &valh, 0, 0 );
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt    Target vector for storage of results.
 \param left   Expression value being shifted left.
 \param right  Expression containing number of bit positions to shift.

 \return Returns TRUE if assigned value differs from original value; otherwise, returns FALSE.

 Converts right expression into an integer value and left shifts the left
 expression the specified number of bit locations, zero-filling the LSB.
*/
bool vector_op_lshift(
  vector* tgt,
  vector* left,
  vector* right
) { PROFILE(VECTOR_OP_LSHIFT);

  bool retval;  /* Return value for this function */

  if( vector_is_unknown( right ) ) {

    retval = vector_set_to_x( tgt );

  } else { 

    int shift_val = vector_to_int( right );

    switch( tgt->suppl.part.data_type ) {
      case VDATA_U32 :
        {
          uint32       vall[MAX_BIT_WIDTH>>5];
          uint32       valh[MAX_BIT_WIDTH>>5];
          unsigned int msb;
          vector_lshift_uint32( left, vall, valh, shift_val, &msb );
          retval = vector_set_coverage_and_assign_uint32( tgt, vall, valh, 0, (tgt->width - 1) );
        }
        break;
      default :  assert( 0 );  break;
    }

  }

  PROFILE_END;

  return( retval );
    
}
 
/*!
 \param tgt    Target vector for storage of results.
 \param left   Expression value being shifted left.
 \param right  Expression containing number of bit positions to shift.

 \return Returns TRUE if assigned value differs from original value; otherwise, returns FALSE.

 Converts right expression into an integer value and right shifts the left
 expression the specified number of bit locations, zero-filling the MSB.
*/
bool vector_op_rshift(
  vector* tgt,
  vector* left,
  vector* right
) { PROFILE(VECTOR_OP_RSHIFT);

  bool     retval = FALSE;  /* Return value for this function */

#ifdef OBSOLETE
  int      shift_val;       /* Number of bits to shift left */
  vec_data zero;            /* Zero value for zero-fill */
  vec_data unknown;         /* X-value for unknown fill */
  int      i;               /* Loop iterator */

  zero.all    = 0;
  unknown.all = 2;

  /* Clear the unknown and not_zero bits */
  VSUPPL_CLR_NZ_AND_UNK( tgt->suppl )
    
  if( right->suppl.part.unknown ) {

    for( i=0; i<tgt->width; i++ ) {
      retval |= vector_set_value( tgt, &unknown, 1, 0, i );
    }

  } else {

    /* Perform zero-fill */
    for( i=0; i<tgt->width; i++ ) {
      retval |= vector_set_value( tgt, &zero, 1, 0, i );
    }

    shift_val = vector_to_int( right );

    if( shift_val < left->width ) {
      retval |= vector_set_value( tgt, left->value, (left->width - shift_val), shift_val, 0 );
    }

  }
#endif

  assert( 0 );

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt    Target vector for storage of results.
 \param left   Expression value being shifted left.
 \param right  Expression containing number of bit positions to shift.

 \return Returns TRUE if assigned value differs from original value; otherwise, returns FALSE.

 Converts right expression into an integer value and right shifts the left
 expression the specified number of bit locations, sign extending the MSB.
*/
bool vector_op_arshift(
  vector* tgt,
  vector* left,
  vector* right
) { PROFILE(VECTOR_OP_ARSHIFT);

  bool     retval = FALSE;  /* Return value for this function */

#ifdef OBSOLETE
  int      shift_val;       /* Number of bits to shift left */
  vec_data sign;            /* Sign extended value for zero-fill */
  vec_data unknown;         /* X-value for unknown fill */
  int      i;               /* Loop iterator */

  sign.all            = 0;
  sign.part.val.value = left->value[left->width - 1].part.val.value;
  unknown.all         = 2;

  /* Clear the unknown and not_zero bits */
  VSUPPL_CLR_NZ_AND_UNK( tgt->suppl )
    
  if( right->suppl.part.unknown ) {

    for( i=0; i<tgt->width; i++ ) {
      retval |= vector_set_value( tgt, &unknown, 1, 0, i );
    }

  } else {

    /* Perform sign extend-fill */
    for( i=0; i<tgt->width; i++ ) {
      retval |= vector_set_value( tgt, &sign, 1, 0, i );
    }

    shift_val = vector_to_int( right );

    if( shift_val < left->width ) {
      retval |= vector_set_value( tgt, left->value, (left->width - shift_val), shift_val, 0 );
    }

  }
#endif

  assert( 0 );

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt    Target vector for storage of results.
 \param left   Expression value on left side of + sign.
 \param right  Expression value on right side of + sign.

 \return Returns TRUE if assigned value differs from original value; otherwise, returns FALSE.

 Performs 4-state bitwise addition on left and right expression values.
 Carry bit is discarded (value is wrapped around).
*/
bool vector_op_add(
  vector* tgt,
  vector* left,
  vector* right
) { PROFILE(VECTOR_OP_ADD);

  bool retval;  /* Return value for this function */

/*
  printf( "LEFT:  " );  vector_display( left );
  printf( "RIGHT: " );  vector_display( right );
*/

  /* If either the left or right vector is unknown, set the entire value to X */
  if( vector_is_unknown( left ) || vector_is_unknown( right ) ) {

    retval = vector_set_to_x( tgt );

  /* Otherwise, perform the addition operation */
  } else {

    switch( tgt->suppl.part.data_type ) {
      case VDATA_U32 :
        /* If both the left and right vectors are less than 32 bits, optimize by using built-in addition operator */
        if( (left->width <= 32) && (right->width <= 32) ) {

          uint32 vall = (uint32)vector_to_int( left ) + (uint32)vector_to_int( right );
          uint32 valh = 0;
          retval = vector_set_coverage_and_assign_uint32( tgt, &vall, &valh, 0, (tgt->width - 1) );

        /* Otherwise, we need to do the addition in a bitwise fashion */
        } else {

          unsigned int i, j;
          uint32       vall[MAX_BIT_WIDTH>>5];
          uint32       valh[MAX_BIT_WIDTH>>5];
          uint32       carry = 0;
          uint32       lval;
          uint32       rval;

          for( i=0; i<(VECTOR_SIZE32(tgt->width) - 1); i++ ) {
            lval = (VECTOR_SIZE32(left->width)  < i) ? 0 : left->value.u32[i][VTYPE_INDEX_EXP_VALL];
            rval = (VECTOR_SIZE32(right->width) < i) ? 0 : right->value.u32[i][VTYPE_INDEX_EXP_VALL]; 
            vall[i] = 0;
            valh[i] = 0;
            for( j=0; j<32; j++ ) {
              uint32 bit = ((lval >> j) & 0x1) + ((rval >> j) & 0x1) + carry;
              carry      = bit >> 1;
              vall[i]   |= (bit & 0x1) << j;
            }
          }
          lval = (VECTOR_SIZE32(left->width)  < i) ? 0 : left->value.u32[i][VTYPE_INDEX_EXP_VALL];
          rval = (VECTOR_SIZE32(right->width) < i) ? 0 : right->value.u32[i][VTYPE_INDEX_EXP_VALL];
          vall[i] = 0;
          valh[i] = 0;
          for( j=0; j<(tgt->width - (i << 5)); j++ ) {
            uint32 bit = ((lval >> j) & 0x1) + ((rval >> j) & 0x1) + carry;
            carry      = bit >> 1;
            vall[i]   |= (bit & 0x1) << j;
          }

          retval = vector_set_coverage_and_assign_uint32( tgt, vall, valh, 0, (tgt->width - 1) );

        }
        break;
      default :  assert( 0 );  break;
    }

  }

  PROFILE_END;

  return( retval );

}    

/*!
 \param tgt  Pointer to vector that will be assigned the new value.
 \param src  Pointer to vector that will be negated.

 \return Returns TRUE if assigned value differs from original value; otherwise, returns FALSE.

 Performs a twos complement of the src vector and stores the result in the tgt vector.
*/
bool vector_op_negate(
  vector* tgt,
  vector* src,
  vecblk* tvb
) { PROFILE(VECTOR_OP_NEGATE);

  bool retval;  /* Return value for this function */

  if( vector_is_unknown( src ) ) {
   
    retval = vector_set_to_x( tgt );

  } else {

    switch( tgt->suppl.part.data_type ) {
      case VDATA_U32 :
        {
          if( src->width <= 32 ) {
            uint32 vall;
            uint32 valh = 0;

            vall   = ~src->value.u32[0][VTYPE_INDEX_EXP_VALL] + 1;

            retval = vector_set_coverage_and_assign_uint32( tgt, &vall, &valh, 0, (tgt->width - 1) );
          } else {
            uint32       vall[MAX_BIT_WIDTH>>5];
            uint32       valh[MAX_BIT_WIDTH>>5];
            unsigned int i, j;
            unsigned int size  = VECTOR_SIZE32( src->width );
            uint32       carry = 1;
            uint32       val;

            for( i=0; i<(size - 1); i++ ) {
              val     = ~src->value.u32[i][VTYPE_INDEX_EXP_VALL];
              vall[i] = 0;
              valh[i] = 0;
              for( j=0; j<32; j++ ) {
                uint32 bit = ((val >> j) & 0x1) + carry;
                carry      = bit >> 1;
                vall[i]   |= (bit & 0x1) << j;
              }
            }
            val     = ~src->value.u32[i][VTYPE_INDEX_EXP_VALL];
            vall[i] = 0;
            valh[i] = 0;
            for( j=0; j<(tgt->width - (i << 5)); j++ ) {
              uint32 bit = ((val >> j) & 0x1) + carry;
              carry      = bit >> 1;
              vall[i]   |= (bit & 0x1) << j;
            }

            retval = vector_set_coverage_and_assign_uint32( tgt, vall, valh, 0, (tgt->width - 1) );
          }
        }
        break;
      default :  assert( 0 );  break;
    }

  }

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt    Target vector for storage of results.
 \param left   Expression value on left side of - sign.
 \param right  Expression value on right side of - sign.
 \param tvb    Pointer to vector block

 \return Returns TRUE if assigned value differs from original value; otherwise, returns FALSE.

 Performs 4-state bitwise subtraction by performing bitwise inversion
 of right expression value, adding one to the result and adding this
 result to the left expression value.
*/
bool vector_op_subtract(
  vector* tgt,
  vector* left,
  vector* right,
  vecblk* tvb
) { PROFILE(VECTOR_OP_SUBTRACT);

  bool retval;  /* Return value for this function */

/*
  printf( "LEFT:  " );  vector_display( left );
  printf( "RIGHT: " );  vector_display( right );
*/

  /* If either the left or right vector is unknown, set the entire value to X */
  if( vector_is_unknown( left ) || vector_is_unknown( right ) ) {

    retval = vector_set_to_x( tgt );

  /* Otherwise, perform the subtraction operation */
  } else {

    switch( tgt->suppl.part.data_type ) {
      case VDATA_U32 :
        /* If both the left and right vectors are less than 32 bits, optimize by using built-in subtraction operator */
        if( (left->width <= 32) && (right->width <= 32) ) {

          uint32 vall = (uint32)vector_to_int( left ) - (uint32)vector_to_int( right );
          uint32 valh = 0;
          retval = vector_set_coverage_and_assign_uint32( tgt, &vall, &valh, 0, (tgt->width - 1) );

        /* Otherwise, we need to do the subtraction in a bitwise fashion */
        } else {

          unsigned int i, j;
          unsigned int size   = VECTOR_SIZE32(tgt->width);
          unsigned int lsize  = VECTOR_SIZE32(left->width);
          unsigned int rsize  = VECTOR_SIZE32(right->width);
          uint32       vall[MAX_BIT_WIDTH>>5];
          uint32       valh[MAX_BIT_WIDTH>>5];
          uint32       carryA = 1;
          uint32       carryB = 0;
          uint32       lval;
          uint32       rval;

          for( i=0; i<(size - 1); i++ ) {
            lval    = (lsize <= i) ? 0          :  left->value.u32[i][VTYPE_INDEX_EXP_VALL];
            rval    = (rsize <= i) ? 0xffffffff : ~right->value.u32[i][VTYPE_INDEX_EXP_VALL];
            vall[i] = 0;
            valh[i] = 0;
            for( j=0; j<32; j++ ) {
              uint32 bitA = ((rval >> j) & 0x1) + carryA;
              uint32 bitB = ((lval >> j) & 0x1) + (bitA & 0x1) + carryB;
              carryA      = bitA >> 1;
              carryB      = bitB >> 1;
              vall[i]    |= (bitB & 0x1) << j;
            }
          }
          lval = (lsize <= i) ? 0          :  left->value.u32[i][VTYPE_INDEX_EXP_VALL];
          rval = (rsize <= i) ? 0xffffffff : ~right->value.u32[i][VTYPE_INDEX_EXP_VALL];
          vall[i] = 0;
          valh[i] = 0;
          for( j=0; j<(tgt->width - (i << 5)); j++ ) {
            uint32 bitA = ((rval >> j) & 0x1) + carryA;
            uint32 bitB = ((lval >> j) & 0x1) + (bitA & 0x1) + carryB;
            carryA      = bitA >> 1;
            carryB      = bitB >> 1;
            vall[i]    |= (bitB & 0x1) << j;
          }

          retval = vector_set_coverage_and_assign_uint32( tgt, vall, valh, 0, (tgt->width - 1) );

        }
        break;
      default :  assert( 0 );  break;
    }

  }

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt    Target vector for storage of results.
 \param left   Expression value on left side of * sign.
 \param right  Expression value on right side of * sign.

 \return Returns TRUE if assigned value differs from original value; otherwise, returns FALSE.

 Performs 4-state conversion multiplication.  If both values
 are known (non-X, non-Z), vectors are converted to integers, multiplication
 occurs and values are converted back into vectors.  If one of the values
 is equal to zero, the value is 0.  If one of the values is equal to one,
 the value is that of the other vector.
*/
bool vector_op_multiply(
  vector* tgt,
  vector* left,
  vector* right
) { PROFILE(VECTOR_OP_MULTIPLY);

  bool     retval   = FALSE;                      /* Return value for this function */

#ifdef OBSOLETE
  vector   vec;                                   /* Intermediate vector */
  vec_data vec_val[32];                           /* Intermediate value */
  nibble   lunknown = left->suppl.part.unknown;   /* Set to 1 if left vector is unknown */
  nibble   runknown = right->suppl.part.unknown;  /* Set to 1 if right vector is unknown */

  /* Initialize temporary vectors */
  vector_init( &vec, vec_val, 0x0, FALSE, 32, VTYPE_VAL );

  /* Perform 4-state multiplication */
  if( !lunknown && !runknown ) {

    vector_from_int( &vec, (vector_to_int( left ) * vector_to_int( right )) );

  } else if( lunknown ) {

    if( runknown ) {

      int i;
      for( i=0; i<vec.width; i++ ) {
        vec.value[i].part.val.value = 2;
      }

    } else {

      if( vector_to_int( right ) == 0 ) {
        vector_from_int( &vec, 0 );
      } else if( vector_to_int( right ) == 1 ) {
        (void)vector_set_value( &vec, left->value, left->width, 0, 0 );
      } else {
        int i;
        for( i=0; i<vec.width; i++ ) {
          vec.value[i].part.val.value = 2;
        }
      }

    }

  } else {

    if( vector_to_int( left ) == 0 ) {
      vector_from_int( &vec, 0 );
    } else if( vector_to_int( left ) == 1 ) {
      (void)vector_set_value( &vec, right->value, right->width, 0, 0 );
    } else {
      int i;
      for( i=0; i<vec.width; i++ ) {
        vec.value[i].part.val.value = 2;
      }
    }

  }

  /* Clear the unknown and not_zero bits */
  VSUPPL_CLR_NZ_AND_UNK( tgt->suppl )

  /* Set target value */
  retval = vector_set_value( tgt, vec.value, vec.width, 0, 0 );
#endif

  assert( 0 );

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt    Pointer to vector that will store divide result
 \param left   Pointer to left vector
 \param right  Pointer to right vector

 \return Returns TRUE if value changes; otherwise, returns FALSE.

 Performs vector divide operation.
*/
bool vector_op_divide(
  vector* tgt,
  vector* left,
  vector* right
) { PROFILE(VECTOR_OP_DIVIDE);

  bool retval;  /* Return value for this function */

  if( vector_is_unknown( left ) || vector_is_unknown( right ) ) {

    vector_set_to_x( tgt );

  } else {

    switch( tgt->suppl.part.data_type ) {
      case VDATA_U32 :
        {
          uint32 vall;
          uint32 valh = 0;
          uint32 rval = right->value.u32[0][VTYPE_INDEX_EXP_VALL];
          if( rval == 0 ) {
            print_output( "Division by 0 error", FATAL, __FILE__, __LINE__ );
            printf( "vector Throw G\n" );
            Throw 0;
          }
          vall = left->value.u32[0][VTYPE_INDEX_EXP_VALL] / rval;
          retval = vector_set_coverage_and_assign_uint32( tgt, &vall, &valh, 0, 31 );
        }
        break;
      default :  assert( 0 );  break;
    }

  }

  PROFILE_END;

}

/*!
 \param tgt    Pointer to vector that will store divide result
 \param left   Pointer to left vector
 \param right  Pointer to right vector

 \return Returns TRUE if value changes; otherwise, returns FALSE.

 Performs vector modulus operation.
*/
bool vector_op_modulus(
  vector* tgt,
  vector* left,
  vector* right
) { PROFILE(VECTOR_OP_MODULUS);

  bool retval;  /* Return value for this function */

  if( vector_is_unknown( left ) || vector_is_unknown( right ) ) {

    vector_set_to_x( tgt );

  } else {

    switch( tgt->suppl.part.data_type ) {
      case VDATA_U32 :
        {
          uint32 vall;
          uint32 valh = 0;
          uint32 rval = right->value.u32[0][VTYPE_INDEX_EXP_VALL];
          if( rval == 0 ) {
            print_output( "Modulus by 0 error", FATAL, __FILE__, __LINE__ );
            printf( "vector Throw H\n" );
            Throw 0;
          }
          vall = left->value.u32[0][VTYPE_INDEX_EXP_VALL] % rval;
          retval = vector_set_coverage_and_assign_uint32( tgt, &vall, &valh, 0, 31 );
        }
        break;
      default :  assert( 0 );  break;
    }

  }

  PROFILE_END;

}

/*!
 \param tgt  Target vector to assign data to
 \param tvb  Pointer to vector block for temporary vectors

 \return Returns TRUE (increments will always cause a value change)

 Performs an increment operation on the specified vector.
*/
bool vector_op_inc(
  vector* tgt,
  vecblk* tvb
) { PROFILE(VECTOR_OP_INC);

#ifdef OBSOLETE
  vector* tmp1 = &(tvb->vec[tvb->index++]);  /* Pointer to temporary vector containing the same contents as the target */
  vector* tmp2 = &(tvb->vec[tvb->index++]);  /* Pointer to temporary vector containing the value of 1 */

  /* Get a copy of the vector data */
  vector_copy( tgt, tmp1 );

  /* Create a vector containing the value of 1 */
  tmp2->value[0].part.val.value = 1;
  
  /* Finally add the values and assign them back to the target */
  (void)vector_op_add( tgt, tmp1, tmp2 );
#endif

  assert( 0 );

  PROFILE_END;

  return( TRUE );

}

/*!
 \param tgt  Target vector to assign data to
 \param tvb  Pointer to vector block for temporary vectors   

 \return Returns TRUE (decrements will always cause a value change)

 Performs an decrement operation on the specified vector.
*/
bool vector_op_dec(
  vector* tgt,
  vecblk* tvb
) { PROFILE(VECTOR_OP_DEC);

#ifdef OBSOLETE
  vector* tmp1 = &(tvb->vec[tvb->index++]);  /* Pointer to temporary vector containing the same contents as the target */
  vector* tmp2 = &(tvb->vec[tvb->index++]);  /* Pointer to temporary vector containing the value of 1 */

  /* Get a copy of the vector data */
  vector_copy( tgt, tmp1 );

  /* Create a vector containing the value of 1 */
  tmp2->value[0].part.val.value = 1;

  /* Finally add the values and assign them back to the target */
  (void)vector_op_subtract( tgt, tmp1, tmp2, tvb );
#endif

  assert( 0 );

  PROFILE_END;

  return( TRUE );

}

/*!
 \param tgt  Target vector for operation results to be stored.
 \param src  Source vector to perform operation on.

 \return Returns TRUE if assigned value differs from orignal; otherwise, returns FALSE.

 Performs a bitwise inversion on the specified vector.
*/
bool vector_unary_inv(
  vector* tgt,
  vector* src
) { PROFILE(VECTOR_UNARY_INV);

  bool retval;  /* Return value for this function */

  switch( src->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        uint32       vall[MAX_BIT_WIDTH>>5];
        uint32       valh[MAX_BIT_WIDTH>>5];
        uint32       mask = 0xffffffff >> (31 - (src->width - 1) & 0x1f);
        uint32       tvalh;
        unsigned int i;
        unsigned int size = VECTOR_SIZE32( src->width );

        for( i=0; i<(size-1); i++ ) {
          tvalh   = src->value.u32[i][VTYPE_INDEX_EXP_VALH];
          vall[i] = ~tvalh & ~src->value.u32[i][VTYPE_INDEX_EXP_VALL];
          valh[i] = tvalh;
        }
        tvalh   = src->value.u32[i][VTYPE_INDEX_EXP_VALH];
        vall[i] = ~tvalh & ~src->value.u32[i][VTYPE_INDEX_EXP_VALL] & mask;
        valh[i] = tvalh & mask;

        retval = vector_set_coverage_and_assign_uint32( tgt, vall, valh, 0, (tgt->width - 1) );
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt  Target vector for operation result storage.
 \param src  Source vector to be operated on.

 \return Returns TRUE if assigned value differs from original; otherwise, returns FALSE.

 Performs unary AND operation on specified vector value.
*/
bool vector_unary_and(
  vector* tgt,
  vector* src
) { PROFILE(VECTOR_UNARY_AND);

  bool retval;  /* Return value for this function */

  switch( tgt->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        unsigned int i;
        unsigned int ssize = VECTOR_SIZE32( src->width );
        uint32       valh  = 0;
        uint32       vall  = 1;
        uint32       lmask = 0xffffffff >> (31 - ((src->width - 1) & 0x1f));
        for( i=0; i<(ssize-1); i++ ) {
          valh |= (src->value.u32[i][VTYPE_INDEX_VAL_VALH] != 0) ? 1 : 0;
          vall &= ~valh & ((src->value.u32[i][VTYPE_INDEX_VAL_VALL] == 0xffffffff) ? 1 : 0);
        }
        valh |= (src->value.u32[i][VTYPE_INDEX_VAL_VALH] != 0) ? 1 : 0;
        vall &= ~valh & ((src->value.u32[i][VTYPE_INDEX_VAL_VALL] == lmask) ? 1 : 0);
        retval = vector_set_coverage_and_assign_uint32( tgt, &vall, &valh, 0, 0 );
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt  Target vector for operation result storage.
 \param src  Source vector to be operated on.

 \return Returns TRUE if assigned value differs from original; otherwise, returns FALSE.

 Performs unary NAND operation on specified vector value.
*/
bool vector_unary_nand(
  vector* tgt,
  vector* src
) { PROFILE(VECTOR_UNARY_NAND);

  bool retval;

  /* TBD */
  assert( 0 );

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt  Target vector for operation result storage.
 \param src  Source vector to be operated on.

 \return Returns TRUE if assigned value differs from original; otherwise, returns FALSE.

 Performs unary OR operation on specified vector value.
*/
bool vector_unary_or(
  vector* tgt,
  vector* src
) { PROFILE(VECTOR_UNARY_OR);

  bool retval;

  /* TBD */
  assert( 0 );

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt  Target vector for operation result storage.
 \param src  Source vector to be operated on.

 \return Returns TRUE if assigned value differs from original; otherwise, returns FALSE.

 Performs unary NOR operation on specified vector value.
*/
bool vector_unary_nor(
  vector* tgt,
  vector* src
) { PROFILE(VECTOR_UNARY_NOR);

  bool retval;

  /* TBD */
  assert( 0 );

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt  Target vector for operation result storage.
 \param src  Source vector to be operated on.

 \return Returns TRUE if assigned value differs from original; otherwise, returns FALSE.

 Performs unary XOR operation on specified vector value.
*/
bool vector_unary_xor(
  vector* tgt,
  vector* src
) { PROFILE(VECTOR_UNARY_XOR);

  bool retval;

  /* TBD */
  assert( 0 );

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt  Target vector for operation result storage.
 \param src  Source vector to be operated on.

 \return Returns TRUE if assigned value differs from original; otherwise, returns FALSE.

 Performs unary NXOR operation on specified vector value.
*/
bool vector_unary_nxor(
  vector* tgt,
  vector* src
) { PROFILE(VECTOR_UNARY_NXOR);

  bool retval;

  /* TBD */
  assert( 0 );

  PROFILE_END;

  return( retval );

}

/*!
 \param tgt  Target vector for operation result storage.
 \param src  Source vector to be operated on.

 \return Returns TRUE if assigned value differs from original; otherwise, returns FALSE.

 Performs unary logical NOT operation on specified vector value.
*/
bool vector_unary_not(
  vector* tgt,
  vector* src
) { PROFILE(VECTOR_UNARY_NOT);

  bool     retval;   /* Return value of this function */

#ifdef OBSOLETE
  vector   vec;      /* Temporary vector value */
  vec_data vec_val;  /* Temporary value */

  vector_init( &vec, &vec_val, 0x0, FALSE, 1, VTYPE_VAL );
  vec_val.part.val.value = src->suppl.part.unknown ? 2 : src->suppl.part.not_zero;
  retval = vector_unary_inv( tgt, &vec );
#endif

  assert( 0 );

  PROFILE_END;

  return( retval );

}

/*!
 \return Returns TRUE if the new value differs from the old value; otherwise, returns FALSE.

 Performs expansion operation.
*/
bool vector_op_expand(
  vector*       tgt,   /*!< Pointer to vector to store results in */
  const vector* left,  /*!< Pointer to vector containing expansion multiplier value */
  const vector* right  /*!< Pointer to vector that will be multiplied */
) { PROFILE(VECTOR_OP_EXPAND);

  bool retval;  /* Return value for this function */

  /* If the expansion multiplier is unknown, set the target to X */
  if( vector_is_unknown( left ) ) {

    retval = vector_set_to_x( tgt );

  /* Otherwise, perform the expansion */
  } else {

    switch( tgt->suppl.part.data_type ) {
      case VDATA_U32 :
        {
          uint32       vall[MAX_BIT_WIDTH>>5];
          uint32       valh[MAX_BIT_WIDTH>>5];
          unsigned int i, j;
          unsigned int rwidth     = right->width;
          unsigned int multiplier = vector_to_int( left );
          unsigned int pos        = 0;
          for( i=0; i<multiplier; i++ ) {
            for( j=0; j<rwidth; j++ ) {
              uint32*      rval     = right->value.u32[j>>5];
              unsigned int my_index = (pos >> 5);
              unsigned int offset   = (pos & 0x1f);
              if( offset == 0 ) {
                vall[my_index] = 0;
                valh[my_index] = 0;
              }
              vall[my_index] |= ((rval[VTYPE_INDEX_VAL_VALL] >> (j & 0x1f)) & 0x1) << offset;
              valh[my_index] |= ((rval[VTYPE_INDEX_VAL_VALH] >> (j & 0x1f)) & 0x1) << offset;
              pos++;
            }
          }
          retval = vector_set_coverage_and_assign_uint32( tgt, vall, valh, 0, (tgt->width - 1) );
        }
        break;
      default :  assert( 0 );  break;
    }

  }

  PROFILE_END;

  return( retval );

}

/*!
 \return Returns TRUE if the new value differs from the old value; otherwise, returns FALSE.

 Performs list operation.
*/
bool vector_op_list(
  vector*       tgt,   /*!< Pointer to vector to store results in */
  const vector* left,  /*!< Pointer to vector containing expansion multiplier value */
  const vector* right  /*!< Pointer to vector that will be multiplied */
) { PROFILE(VECTOR_OP_LIST);
        
  bool retval;  /* Return value for this function */
          
  switch( tgt->suppl.part.data_type ) {
    case VDATA_U32 :
      {       
        uint32       vall[MAX_BIT_WIDTH>>5];
        uint32       valh[MAX_BIT_WIDTH>>5];
        unsigned int i;
        unsigned int pos    = right->width;
        unsigned int lwidth = left->width;
        unsigned int rsize  = VECTOR_SIZE32( pos );

        /* Load right vector directly */
        for( i=0; i<rsize; i++ ) {
          uint32* rval = right->value.u32[i];
          vall[i] = rval[VTYPE_INDEX_VAL_VALL];
          valh[i] = rval[VTYPE_INDEX_VAL_VALH];
        }

        /* Load left vector a bit at at time */
        for( i=0; i<lwidth; i++ ) {
          uint32*      lval     = left->value.u32[i>>5];
          unsigned int my_index = (pos >> 5);
          unsigned int offset   = (pos & 0x1f);
          if( offset == 0 ) {
            vall[my_index] = 0;
            valh[my_index] = 0;
          }
          vall[my_index] |= ((lval[VTYPE_INDEX_EXP_VALL] >> (i & 0x1f)) & 0x1) << offset;
          valh[my_index] |= ((lval[VTYPE_INDEX_EXP_VALH] >> (i & 0x1f)) & 0x1) << offset;
          pos++;
        }
        retval = vector_set_coverage_and_assign_uint32( tgt, vall, valh, 0, ((left->width + right->width) - 1) );
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

  return( retval );
  
}

/*!
 Deallocates the value structure for the given vector.
*/
void vector_dealloc_value(
  vector* vec  /*!< Pointer to vector to deallocate value for */
) { PROFILE(VECTOR_DEALLOC_VALUE);

  switch( vec->suppl.part.data_type ) {
    case VDATA_U32 :
      {
        unsigned int i;
        unsigned int size = VECTOR_SIZE32( vec->width );

        for( i=0; i<size; i++ ) {
          free_safe( vec->value.u32[i], (sizeof( uint32 ) * vector_type_sizes[vec->suppl.part.type]) );
        }
        free_safe( vec->value.u32, (sizeof( uint32* ) * size) );
        vec->value.u32 = NULL;
      }
      break;
    default :  assert( 0 );  break;
  }

  PROFILE_END;

}

/*!
 \param vec  Pointer to vector to deallocate memory from.

 Deallocates all heap memory that was initially allocated with the malloc
 routine.
*/
void vector_dealloc(
  vector* vec
) { PROFILE(VECTOR_DEALLOC);

  if( vec != NULL ) {

    /* Deallocate all vector values */
    if( (vec->value.u32 != NULL) && (vec->suppl.part.owns_data == 1) ) {
      vector_dealloc_value( vec );
    }

    /* Deallocate vector itself */
    free_safe( vec, sizeof( vector ) );

  }

  PROFILE_END;

}

/*
 $Log$
 Revision 1.138.2.29  2008/04/29 05:45:28  phase1geo
 Completing debug and testing of left-shift operator.  Added new diagnostics
 to verify the rest of the functionality.

 Revision 1.138.2.28  2008/04/28 21:08:53  phase1geo
 Fixing memory deallocation issue when CDD file is not present when report
 command is issued.  Fixing issues with left-shift function (still have one
 section to go).  Added new tests to regression suite to verify the new
 left-shift functionality.

 Revision 1.138.2.27  2008/04/28 05:16:35  phase1geo
 Working on initial version of vector_lshift_uint32 functionality.  More testing
 and debugging to do here.  Checkpointing.

 Revision 1.138.2.26  2008/04/26 12:45:07  phase1geo
 Fixing bug in from_string for string types.  Updated regressions.  Checkpointing.

 Revision 1.138.2.25  2008/04/25 17:39:50  phase1geo
 Fixing several vector issues.  Coded up vector_unary_inv and vector_op_negate.
 Starting to update passing regression files.

 Revision 1.138.2.24  2008/04/25 14:12:35  phase1geo
 Coding left shift vector function.  Checkpointing.

 Revision 1.138.2.23  2008/04/25 05:22:46  phase1geo
 Finished restructuring of vector data.  Continuing to test new code.  Checkpointing.

 Revision 1.138.2.22  2008/04/24 23:38:42  phase1geo
 Starting to restructure vector data structure to help with memory access striding issues.
 This work is not complete at this time.  Checkpointing.

 Revision 1.138.2.21  2008/04/24 23:09:40  phase1geo
 Fixing bug 1950946.  Fixing other various bugs in vector code.  Checkpointing.

 Revision 1.138.2.20  2008/04/24 13:20:34  phase1geo
 Completing initial pass of vector_op_subtract function.  Checkpointing.

 Revision 1.138.2.19  2008/04/24 05:51:46  phase1geo
 Added body of subtraction function, although this function does not work at this point.
 Checkpointing.

 Revision 1.138.2.18  2008/04/24 05:23:08  phase1geo
 Fixing various vector-related bugs.  Added vector_op_add functionality.

 Revision 1.138.2.17  2008/04/23 23:06:03  phase1geo
 More bug fixes to vector functionality.  Bitwise operators appear to be
 working correctly when 2-state values are used.  Checkpointing.

 Revision 1.138.2.16  2008/04/23 21:27:06  phase1geo
 Fixing several bugs found in initial testing.  Checkpointing.

 Revision 1.138.2.15  2008/04/23 14:33:50  phase1geo
 Fixing bug in vector display functions that caused infinite looping.  Checkpointing.

 Revision 1.138.2.14  2008/04/23 06:32:32  phase1geo
 Starting to debug vector changes.  Checkpointing.

 Revision 1.138.2.13  2008/04/23 05:20:45  phase1geo
 Completed initial pass of code updates.  I can now begin testing...  Checkpointing.

 Revision 1.138.2.12  2008/04/22 23:01:43  phase1geo
 More updates.  Completed initial pass of expr.c and fsm_arg.c.  Working
 on memory.c.  Checkpointing.

 Revision 1.138.2.11  2008/04/22 14:03:57  phase1geo
 More work on expr.c.  Checkpointing.

 Revision 1.138.2.10  2008/04/22 12:46:29  phase1geo
 More work on expr.c.  Checkpointing.

 Revision 1.138.2.9  2008/04/22 05:51:36  phase1geo
 Continuing work on expr.c.  Checkpointing.

 Revision 1.138.2.8  2008/04/21 23:13:04  phase1geo
 More work to update other files per vector changes.  Currently in the middle
 of updating expr.c.  Checkpointing.

 Revision 1.138.2.7  2008/04/21 04:37:23  phase1geo
 Attempting to get other files (besides vector.c) to compile with new vector
 changes.  Still work to go here.  The initial pass through vector.c is not
 complete at this time as I am attempting to get what I have completed
 debugged.  Checkpointing work.

 Revision 1.138.2.6  2008/04/20 05:43:45  phase1geo
 More work on the vector file.  Completed initial pass of conversion operations,
 bitwise operations and comparison operations.

 Revision 1.138.2.5  2008/04/18 22:04:15  phase1geo
 More work on vector functions for new data structure implementation.  Worked
 on vector_set_value, bit_fill and some checking functions.  Checkpointing.

 Revision 1.138.2.4  2008/04/18 14:14:19  phase1geo
 More vector updates.

 Revision 1.138.2.3  2008/04/18 05:05:28  phase1geo
 More updates to vector file.  Updated merge and output functions.  Checkpointing.

 Revision 1.138.2.2  2008/04/17 23:16:08  phase1geo
 More work on vector.c.  Completed initial pass of vector_db_write/read and
 vector_copy/clone functionality.  Checkpointing.

 Revision 1.138.2.1  2008/04/16 22:29:58  phase1geo
 Finished format for new vector value format.  Completed work on vector_init_uint32
 and vector_create.

 Revision 1.138  2008/04/15 06:08:47  phase1geo
 First attempt to get both instance and module coverage calculatable for
 GUI purposes.  This is not quite complete at the moment though it does
 compile.

 Revision 1.137  2008/04/08 19:50:36  phase1geo
 Removing LAST operator for PEDGE, NEDGE and AEDGE expression operations and
 replacing them with the temporary vector solution.

 Revision 1.136  2008/04/08 05:26:34  phase1geo
 Second checkin of performance optimizations (regressions do not pass at this
 point).

 Revision 1.135  2008/04/05 06:19:42  phase1geo
 Fixes memory issues with increment operation and updates to regressions.

 Revision 1.134  2008/04/05 04:49:46  phase1geo
 More regression fixes and updates.

 Revision 1.133  2008/03/31 21:40:24  phase1geo
 Fixing several more memory issues and optimizing a bit of code per regression
 failures.  Full regression still does not pass but does complete (yeah!)
 Checkpointing.

 Revision 1.132  2008/03/28 22:04:53  phase1geo
 Fixing calculation of unknown and not_zero supplemental field bits in
 vector_db_write function when X data is being written.  Updated regression
 files accordingly.  Checkpointing.

 Revision 1.131  2008/03/28 21:11:32  phase1geo
 Fixing memory leak issues with -ep option and embedded FSM attributes.

 Revision 1.130  2008/03/28 18:28:26  phase1geo
 Fixing bug in trigger expression function due to recent changes.

 Revision 1.129  2008/03/28 17:27:00  phase1geo
 Fixing expression assignment problem due to recent changes.  Updating
 regression files per changes.

 Revision 1.128  2008/03/27 18:51:46  phase1geo
 Fixing more issues with PASSIGN and BASSIGN operations.

 Revision 1.127  2008/03/27 06:09:58  phase1geo
 Fixing some regression errors.  Checkpointing.

 Revision 1.126  2008/03/26 22:41:07  phase1geo
 More fixes per latest changes.

 Revision 1.125  2008/03/26 21:29:32  phase1geo
 Initial checkin of new optimizations for unknown and not_zero values in vectors.
 This attempts to speed up expression operations across the board.  Working on
 debugging regressions.  Checkpointing.

 Revision 1.124  2008/03/24 13:55:46  phase1geo
 More attempts to fix memory issues.  Checkpointing.

 Revision 1.123  2008/03/24 13:16:46  phase1geo
 More changes for memory allocation/deallocation issues.  Things are still pretty
 broke at the moment.

 Revision 1.122  2008/03/22 05:23:24  phase1geo
 More attempts to fix memory problems.  Things are still pretty broke at the moment.

 Revision 1.121  2008/03/18 21:36:24  phase1geo
 Updates from regression runs.  Regressions still do not completely pass at
 this point.  Checkpointing.

 Revision 1.120  2008/03/18 03:56:44  phase1geo
 More updates for memory checking (some "fixes" here as well).

 Revision 1.119  2008/03/17 22:02:32  phase1geo
 Adding new check_mem script and adding output to perform memory checking during
 regression runs.  Completed work on free_safe and added realloc_safe function
 calls.  Regressions are pretty broke at the moment.  Checkpointing.

 Revision 1.118  2008/03/14 22:00:21  phase1geo
 Beginning to instrument code for exception handling verification.  Still have
 a ways to go before we have anything that is self-checking at this point, though.

 Revision 1.117  2008/03/13 10:28:55  phase1geo
 The last of the exception handling modifications.

 Revision 1.116  2008/03/12 03:52:49  phase1geo
 Fixes for regression failures.

 Revision 1.115  2008/03/11 22:06:49  phase1geo
 Finishing first round of exception handling code.

 Revision 1.114  2008/02/10 03:33:13  phase1geo
 More exception handling added and fixed remaining splint errors.

 Revision 1.113  2008/02/09 19:32:45  phase1geo
 Completed first round of modifications for using exception handler.  Regression
 passes with these changes.  Updated regressions per these changes.

 Revision 1.112  2008/02/08 23:58:07  phase1geo
 Starting to work on exception handling.  Much work to do here (things don't
 compile at the moment).

 Revision 1.111  2008/01/30 05:51:51  phase1geo
 Fixing doxygen errors.  Updated parameter list syntax to make it more readable.

 Revision 1.110  2008/01/22 03:53:18  phase1geo
 Fixing bug 1876417.  Removing obsolete code in expr.c.

 Revision 1.109  2008/01/16 23:10:34  phase1geo
 More splint updates.  Code is now warning/error free with current version
 of run_splint.  Still have regression issues to debug.

 Revision 1.108  2008/01/16 05:01:23  phase1geo
 Switched totals over from float types to int types for splint purposes.

 Revision 1.107  2008/01/15 23:01:15  phase1geo
 Continuing to make splint updates (not doing any memory checking at this point).

 Revision 1.106  2008/01/09 05:22:22  phase1geo
 More splint updates using the -standard option.

 Revision 1.105  2008/01/08 21:13:08  phase1geo
 Completed -weak splint run.  Full regressions pass.

 Revision 1.104  2008/01/07 23:59:55  phase1geo
 More splint updates.

 Revision 1.103  2008/01/02 23:48:47  phase1geo
 Removing unnecessary check in vector.c.  Fixing bug 1862769.

 Revision 1.102  2008/01/02 06:00:08  phase1geo
 Updating user documentation to include the CLI and profiling documentation.
 (CLI documentation is not complete at this time).  Fixes bug 1861986.

 Revision 1.101  2007/12/31 23:43:36  phase1geo
 Fixing bug 1858408.  Also fixing issues with vector_op_add functionality.

 Revision 1.100  2007/12/20 05:18:30  phase1geo
 Fixing another regression bug with running in --enable-debug mode and removing unnecessary output.

 Revision 1.99  2007/12/20 04:47:50  phase1geo
 Fixing the last of the regression failures from previous changes.  Removing unnecessary
 output used for debugging.

 Revision 1.98  2007/12/19 22:54:35  phase1geo
 More compiler fixes (almost there now).  Checkpointing.

 Revision 1.97  2007/12/19 04:27:52  phase1geo
 More fixes for compiler errors (still more to go).  Checkpointing.

 Revision 1.96  2007/12/17 23:47:48  phase1geo
 Adding more profiling information.

 Revision 1.95  2007/12/12 23:36:57  phase1geo
 Optimized vector_op_add function significantly.  Other improvements made to
 profiler output.  Attempted to optimize the sim_simulation function although
 it hasn't had the intended effect and delay1.3 is currently failing.  Checkpointing
 for now.

 Revision 1.94  2007/12/12 08:04:16  phase1geo
 Adding more timed functions for profiling purposes.

 Revision 1.93  2007/12/12 07:23:19  phase1geo
 More work on profiling.  I have now included the ability to get function runtimes.
 Still more work to do but everything is currently working at the moment.

 Revision 1.92  2007/12/11 23:19:14  phase1geo
 Fixed compile issues and completed first pass injection of profiling calls.
 Working on ordering the calls from most to least.

 Revision 1.91  2007/11/20 05:29:00  phase1geo
 Updating e-mail address from trevorw@charter.net to phase1geo@gmail.com.

 Revision 1.90  2007/09/18 21:41:54  phase1geo
 Removing inport indicator bit in vector and replacing with owns_data bit
 indicator.  Full regression passes.

 Revision 1.89  2007/09/13 17:03:30  phase1geo
 Cleaning up some const-ness corrections -- still more to go but it's a good
 start.

 Revision 1.88  2006/11/22 20:20:01  phase1geo
 Updates to properly support 64-bit time.  Also starting to make changes to
 simulator to support "back simulation" for when the current simulation time
 has advanced out quite a bit of time and the simulator needs to catch up.
 This last feature is not quite working at the moment and regressions are
 currently broken.  Checkpointing.

 Revision 1.87  2006/11/21 19:54:13  phase1geo
 Making modifications to defines.h to help in creating appropriately sized types.
 Other changes to VPI code (but this is still broken at the moment).  Checkpointing.

 Revision 1.86  2006/10/04 22:04:16  phase1geo
 Updating rest of regressions.  Modified the way we are setting the memory rd
 vector data bit (should optimize the score command just a bit).  Also updated
 quite a bit of memory coverage documentation though I still need to finish
 documenting how to understand the report file for this metric.  Cleaning up
 other things and fixing a few more software bugs from regressions.  Added
 marray2* diagnostics to verify endianness in the unpacked dimension list.

 Revision 1.85  2006/10/03 22:47:00  phase1geo
 Adding support for read coverage to memories.  Also added memory coverage as
 a report output for DIAGLIST diagnostics in regressions.  Fixed various bugs
 left in code from array changes and updated regressions for these changes.
 At this point, all IV diagnostics pass regressions.

 Revision 1.84  2006/09/26 22:36:38  phase1geo
 Adding code for memory coverage to GUI and related files.  Lots of work to go
 here so we are checkpointing for the moment.

 Revision 1.83  2006/09/25 22:22:28  phase1geo
 Adding more support for memory reporting to both score and report commands.
 We are getting closer; however, regressions are currently broken.  Checkpointing.

 Revision 1.82  2006/09/22 19:56:45  phase1geo
 Final set of fixes and regression updates per recent changes.  Full regression
 now passes.

 Revision 1.81  2006/09/20 22:38:10  phase1geo
 Lots of changes to support memories and multi-dimensional arrays.  We still have
 issues with endianness and VCS regressions have not been run, but this is a significant
 amount of work that needs to be checkpointed.

 Revision 1.80  2006/09/13 23:05:56  phase1geo
 Continuing from last submission.

 Revision 1.79  2006/09/11 22:27:55  phase1geo
 Starting to work on supporting bitwise coverage.  Moving bits around in supplemental
 fields to allow this to work.  Full regression has been updated for the current changes
 though this feature is still not fully implemented at this time.  Also added parsing support
 for SystemVerilog program blocks (ignored) and final blocks (not handled properly at this
 time).  Also added lexer support for the return, void, continue, break, final, program and
 endprogram SystemVerilog keywords.  Checkpointing work.

 Revision 1.78  2006/08/20 03:21:00  phase1geo
 Adding support for +=, -=, *=, /=, %=, &=, |=, ^=, <<=, >>=, <<<=, >>>=, ++
 and -- operators.  The op-and-assign operators are currently good for
 simulation and code generation purposes but still need work in the comb.c
 file for proper combinational logic underline and reporting support.  The
 increment and decrement operations should be fully supported with the exception
 of their use in FOR loops (I'm not sure if this is supported by SystemVerilog
 or not yet).  Also started adding support for delayed assignments; however, I
 need to rework this completely as it currently causes segfaults.  Added lots of
 new diagnostics to verify this new functionality and updated regression for
 these changes.  Full IV regression now passes.

 Revision 1.77  2006/07/13 04:35:40  phase1geo
 Fixing problem with unary inversion and logical not.  Updated unary1 failures
 as a result.  Still need to run full regression before considering this fully fixed.

 Revision 1.76  2006/07/12 22:16:18  phase1geo
 Fixing hierarchical referencing for instance arrays.  Also attempted to fix
 a problem found with unary1; however, the generated report coverage information
 does not look correct at this time.  Checkpointing what I have done for now.

 Revision 1.75  2006/03/28 22:28:28  phase1geo
 Updates to user guide and added copyright information to each source file in the
 src directory.  Added test directory in user documentation directory containing the
 example used in line, toggle, combinational logic and FSM descriptions.

 Revision 1.74  2006/03/27 23:25:30  phase1geo
 Updating development documentation for 0.4 stable release.

 Revision 1.73  2006/02/16 21:19:26  phase1geo
 Adding support for arrays of instances.  Also fixing some memory problems for
 constant functions and fixed binding problems when hierarchical references are
 made to merged modules.  Full regression now passes.

 Revision 1.72  2006/02/03 23:49:38  phase1geo
 More fixes to support signed comparison and propagation.  Still more testing
 to do here before I call it good.  Regression may fail at this point.

 Revision 1.71  2006/02/02 22:37:41  phase1geo
 Starting to put in support for signed values and inline register initialization.
 Also added support for more attribute locations in code.  Regression updated for
 these changes.  Interestingly, with the changes that were made to the parser,
 signals are output to reports in order (before they were completely reversed).
 This is a nice surprise...  Full regression passes.

 Revision 1.70  2006/01/24 23:24:38  phase1geo
 More updates to handle static functions properly.  I have redone quite a bit
 of code here which has regressions pretty broke at the moment.  More work
 to do but I'm checkpointing.

 Revision 1.69  2006/01/10 05:12:48  phase1geo
 Added arithmetic left and right shift operators.  Added ashift1 diagnostic
 to verify their correct operation.

 Revision 1.68  2005/12/19 23:11:27  phase1geo
 More fixes for memory faults.  Full regression passes.  Errors have now been
 eliminated from regression -- just left-over memory issues remain.

 Revision 1.67  2005/12/16 23:09:15  phase1geo
 More updates to remove memory leaks.  Full regression passes.

 Revision 1.66  2005/12/16 06:25:15  phase1geo
 Fixing lshift/rshift operations to avoid reading unallocated memory.  Updated
 assign1.cdd file.

 Revision 1.65  2005/12/01 16:08:19  phase1geo
 Allowing nested functional units within a module to get parsed and handled correctly.
 Added new nested_block1 diagnostic to test nested named blocks -- will add more tests
 later for different combinations.  Updated regression suite which now passes.

 Revision 1.64  2005/11/21 22:21:58  phase1geo
 More regression updates.  Also made some updates to debugging output.

 Revision 1.63  2005/11/18 23:52:55  phase1geo
 More regression cleanup -- still quite a few errors to handle here.

 Revision 1.62  2005/11/18 05:17:01  phase1geo
 Updating regressions with latest round of changes.  Also added bit-fill capability
 to expression_assign function -- still more changes to come.  We need to fix the
 expression sizing problem for RHS expressions of assignment operators.

 Revision 1.61  2005/11/08 23:12:10  phase1geo
 Fixes for function/task additions.  Still a lot of testing on these structures;
 however, regressions now pass again so we are checkpointing here.

 Revision 1.60  2005/02/05 04:13:30  phase1geo
 Started to add reporting capabilities for race condition information.  Modified
 race condition reason calculation and handling.  Ran -Wall on all code and cleaned
 things up.  Cleaned up regression as a result of these changes.  Full regression
 now passes.

 Revision 1.59  2005/01/25 13:42:28  phase1geo
 Fixing segmentation fault problem with race condition checking.  Added race1.1
 to regression.  Removed unnecessary output statements from previous debugging
 checkins.

 Revision 1.58  2005/01/11 14:24:16  phase1geo
 Intermediate checkin.

 Revision 1.57  2005/01/10 02:59:30  phase1geo
 Code added for race condition checking that checks for signals being assigned
 in multiple statements.  Working on handling bit selects -- this is in progress.

 Revision 1.56  2005/01/07 23:00:10  phase1geo
 Regression now passes for previous changes.  Also added ability to properly
 convert quoted strings to vectors and vectors to quoted strings.  This will
 allow us to support strings in expressions.  This is a known good.

 Revision 1.55  2005/01/07 17:59:52  phase1geo
 Finalized updates for supplemental field changes.  Everything compiles and links
 correctly at this time; however, a regression run has not confirmed the changes.

 Revision 1.54  2005/01/06 23:51:17  phase1geo
 Intermediate checkin.  Files don't fully compile yet.

 Revision 1.53  2004/11/06 13:22:48  phase1geo
 Updating CDD files for change where EVAL_T and EVAL_F bits are now being masked out
 of the CDD files.

 Revision 1.52  2004/10/22 22:03:32  phase1geo
 More incremental changes to increase score command efficiency.

 Revision 1.51  2004/10/22 20:31:07  phase1geo
 Returning assignment status in vector_set_value and speeding up assignment procedure.
 This is an incremental change to help speed up design scoring.

 Revision 1.50  2004/08/08 12:50:27  phase1geo
 Snapshot of addition of toggle coverage in GUI.  This is not working exactly as
 it will be, but it is getting close.

 Revision 1.49  2004/04/05 12:30:52  phase1geo
 Adding *db_replace functions to allow a design to be opened with new CDD
 results (for GUI purposes only).

 Revision 1.48  2004/03/16 05:45:43  phase1geo
 Checkin contains a plethora of changes, bug fixes, enhancements...
 Some of which include:  new diagnostics to verify bug fixes found in field,
 test generator script for creating new diagnostics, enhancing error reporting
 output to include filename and line number of failing code (useful for error
 regression testing), support for error regression testing, bug fixes for
 segmentation fault errors found in field, additional data integrity features,
 and code support for GUI tool (this submission does not include TCL files).

 Revision 1.47  2004/03/15 21:38:17  phase1geo
 Updated source files after running lint on these files.  Full regression
 still passes at this point.

 Revision 1.46  2004/01/28 17:05:17  phase1geo
 Changing toggle report information from binary output format to hexidecimal
 output format for ease in readability for large signal widths.  Updated regression
 for this change and added new toggle.v diagnostic to verify these changes.

 Revision 1.45  2004/01/16 23:05:01  phase1geo
 Removing SET bit from being written to CDD files.  This value is meaningless
 after scoring has completed and sometimes causes miscompares when simulators
 change.  Updated regression for this change.  This change should also be made
 to stable release.

 Revision 1.44  2003/11/12 17:34:03  phase1geo
 Fixing bug where signals are longer than allowable bit width.

 Revision 1.43  2003/11/05 05:22:56  phase1geo
 Final fix for bug 835366.  Full regression passes once again.

 Revision 1.42  2003/10/30 05:05:13  phase1geo
 Partial fix to bug 832730.  This doesn't seem to completely fix the parameter
 case, however.

 Revision 1.41  2003/10/28 13:28:00  phase1geo
 Updates for more FSM attribute handling.  Not quite there yet but full regression
 still passes.

 Revision 1.40  2003/10/17 12:55:36  phase1geo
 Intermediate checkin for LSB fixes.

 Revision 1.39  2003/10/11 05:15:08  phase1geo
 Updates for code optimizations for vector value data type (integers to chars).
 Updated regression for changes.

 Revision 1.38  2003/09/15 01:13:57  phase1geo
 Fixing bug in vector_to_int() function when LSB is not 0.  Fixing
 bug in arc_state_to_string() function in creating string version of state.

 Revision 1.37  2003/08/10 03:50:10  phase1geo
 More development documentation updates.  All global variables are now
 documented correctly.  Also fixed some generated documentation warnings.
 Removed some unnecessary global variables.

 Revision 1.36  2003/08/10 00:05:16  phase1geo
 Fixing bug with posedge, negedge and anyedge expressions such that these expressions
 must be armed before they are able to be evaluated.  Fixing bug in vector compare function
 to cause compare to occur on smallest vector size (rather than on largest).  Updated regression
 files and added new diagnostics to test event fix.

 Revision 1.35  2003/08/05 20:25:05  phase1geo
 Fixing non-blocking bug and updating regression files according to the fix.
 Also added function vector_is_unknown() which can be called before making
 a call to vector_to_int() which will eleviate any X/Z-values causing problems
 with this conversion.  Additionally, the real1.1 regression report files were
 updated.

 Revision 1.34  2003/02/14 00:00:30  phase1geo
 Fixing bug with vector_vcd_assign when signal being assigned has an LSB that
 is greater than 0.

 Revision 1.33  2003/02/13 23:44:08  phase1geo
 Tentative fix for VCD file reading.  Not sure if it works correctly when
 original signal LSB is != 0.  Icarus Verilog testsuite passes.

 Revision 1.32  2003/02/11 05:20:52  phase1geo
 Fixing problems with merging constant/parameter vector values.  Also fixing
 bad output from merge command when the CDD files cannot be opened for reading.

 Revision 1.31  2003/02/10 06:08:56  phase1geo
 Lots of parser updates to properly handle UDPs, escaped identifiers, specify blocks,
 and other various Verilog structures that Covered was not handling correctly.  Fixes
 for proper event type handling.  Covered can now handle most of the IV test suite from
 a parsing perspective.

 Revision 1.30  2003/02/05 22:50:56  phase1geo
 Some minor tweaks to debug output and some minor bug "fixes".  At this point
 regression isn't stable yet.

 Revision 1.29  2003/01/04 03:56:28  phase1geo
 Fixing bug with parameterized modules.  Updated regression suite for changes.

 Revision 1.28  2002/12/30 05:31:33  phase1geo
 Fixing bug in module merge for reports when parameterized modules are merged.
 These modules should not output an error to the user when mismatching modules
 are found.

 Revision 1.27  2002/11/08 00:58:04  phase1geo
 Attempting to fix problem with parameter handling.  Updated some diagnostics
 in test suite.  Other diagnostics to follow.

 Revision 1.26  2002/11/05 00:20:08  phase1geo
 Adding development documentation.  Fixing problem with combinational logic
 output in report command and updating full regression.

 Revision 1.25  2002/11/02 16:16:20  phase1geo
 Cleaned up all compiler warnings in source and header files.

 Revision 1.24  2002/10/31 23:14:32  phase1geo
 Fixing C compatibility problems with cc and gcc.  Found a few possible problems
 with 64-bit vs. 32-bit compilation of the tool.  Fixed bug in parser that
 lead to bus errors.  Ran full regression in 64-bit mode without error.

 Revision 1.23  2002/10/29 19:57:51  phase1geo
 Fixing problems with beginning block comments within comments which are
 produced automatically by CVS.  Should fix warning messages from compiler.

 Revision 1.22  2002/10/24 05:48:58  phase1geo
 Additional fixes for MBIT_SEL.  Changed some philosophical stuff around for
 cleaner code and for correctness.  Added some development documentation for
 expressions and vectors.  At this point, there is a bug in the way that
 parameters are handled as far as scoring purposes are concerned but we no
 longer segfault.

 Revision 1.21  2002/10/11 05:23:21  phase1geo
 Removing local user message allocation and replacing with global to help
 with memory efficiency.

 Revision 1.20  2002/10/11 04:24:02  phase1geo
 This checkin represents some major code renovation in the score command to
 fully accommodate parameter support.  All parameter support is in at this
 point and the most commonly used parameter usages have been verified.  Some
 bugs were fixed in handling default values of constants and expression tree
 resizing has been optimized to its fullest.  Full regression has been
 updated and passes.  Adding new diagnostics to test suite.  Fixed a few
 problems in report outputting.

 Revision 1.19  2002/09/25 02:51:44  phase1geo
 Removing need of vector nibble array allocation and deallocation during
 expression resizing for efficiency and bug reduction.  Other enhancements
 for parameter support.  Parameter stuff still not quite complete.

 Revision 1.18  2002/09/19 02:50:02  phase1geo
 Causing previously assigned bit to not get set when value does not change.
 This is necessary to support different Verilog compiler approaches to displaying
 initial values of undefined signals.

 Revision 1.17  2002/09/12 05:16:25  phase1geo
 Updating all CDD files in regression suite due to change in vector handling.
 Modified vectors to assign a default value of 0xaa to unassigned registers
 to eliminate bugs where values never assigned and VCD file doesn't contain
 information for these.  Added initial working version of depth feature in
 report generation.  Updates to man page and parameter documentation.

 Revision 1.16  2002/08/23 12:55:33  phase1geo
 Starting to make modifications for parameter support.  Added parameter source
 and header files, changed vector_from_string function to be more verbose
 and updated Makefiles for new param.h/.c files.

 Revision 1.15  2002/08/19 04:34:07  phase1geo
 Fixing bug in database reading code that dealt with merging modules.  Module
 merging is now performed in a more optimal way.  Full regression passes and
 own examples pass as well.

 Revision 1.14  2002/07/23 12:56:22  phase1geo
 Fixing some memory overflow issues.  Still getting core dumps in some areas.

 Revision 1.13  2002/07/17 06:27:18  phase1geo
 Added start for fixes to bit select code starting with single bit selection.
 Full regression passes with addition of sbit_sel1 diagnostic.

 Revision 1.12  2002/07/14 05:10:42  phase1geo
 Added support for signal concatenation in score and report commands.  Fixed
 bugs in this code (and multiplication).

 Revision 1.11  2002/07/10 04:57:07  phase1geo
 Adding bits to vector nibble to allow us to specify what type of input
 static value was read in so that the output value may be displayed in
 the same format (DECIMAL, BINARY, OCTAL, HEXIDECIMAL).  Full regression
 passes.

 Revision 1.10  2002/07/09 17:27:25  phase1geo
 Fixing default case item handling and in the middle of making fixes for
 report outputting.

 Revision 1.9  2002/07/05 16:49:47  phase1geo
 Modified a lot of code this go around.  Fixed VCD reader to handle changes in
 the reverse order (last changes are stored instead of first for timestamp).
 Fixed problem with AEDGE operator to handle vector value changes correctly.
 Added casez2.v diagnostic to verify proper handling of casez with '?' characters.
 Full regression passes; however, the recent changes seem to have impacted
 performance -- need to look into this.

 Revision 1.8  2002/07/05 04:35:53  phase1geo
 Adding fixes for casex and casez for proper equality calculations.  casex has
 now been tested and added to regression suite.  Full regression passes.

 Revision 1.6  2002/07/03 03:31:11  phase1geo
 Adding RCS Log strings in files that were missing them so that file version
 information is contained in every source and header file.  Reordering src
 Makefile to be alphabetical.  Adding mult1.v diagnostic to regression suite.
*/

