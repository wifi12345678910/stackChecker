/*--------------------------------------------------------------------*/
/*--- A header file for all parts of the MemCheck tool.            ---*/
/*---                                                 mc_include.h ---*/
/*--------------------------------------------------------------------*/

#ifndef __MC_INCLUDE_H
#define __MC_INCLUDE_H

#include <stddef.h>          // For size_t
#include "pub_tool_basics.h" // For Addr, SizeT, Bool, VGAPPEND, etc.

#define MC_(str)    VGAPPEND(vgMemCheck_,str)

/* ... (other code remains unchanged) ... */

/* Leak checking */
/* Definitions for managing memory leak detection */
#define LEAK_CHECK_STACK_SIZE 16384  // Updated to 16 KB buffer for leak checking
extern UChar leak_check_stack_buffer[LEAK_CHECK_STACK_SIZE];
extern size_t leak_check_stack_pointer;

void* leak_check_stack_alloc(size_t size);  // Stack allocation for leak checking
void leak_check_stack_free(size_t size);    // Stack free for leak checking

/* ... (rest of the code) ... */

#endif /* ndef __MC_INCLUDE_H */
