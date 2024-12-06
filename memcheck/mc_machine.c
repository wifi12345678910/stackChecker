/*--------------------------------------------------------------------*/
/*--- Contains machine-specific (guest-state-layout-specific)      ---*/
/*--- support for origin tracking.                                 ---*/
/*---                                                 mc_machine.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of MemCheck, a heavyweight Valgrind tool for
   detecting memory errors.

   Copyright (C) 2008-2017 OpenWorks Ltd
      info@open-works.co.uk

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/>.

   The GNU General Public License is contained in the file COPYING.

   Neither the names of the U.S. Department of Energy nor the
   University of California nor the names of its contributors may be
   used to endorse or promote products derived from this software
   without prior written permission.
*/

#include "pub_tool_basics.h"
#include "pub_tool_poolalloc.h"     // For mc_include.h
#include "pub_tool_hashtable.h"     // For mc_include.h
#include "pub_tool_libcassert.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_tooliface.h"
#include "pub_tool_guest.h"         // VexGuestArchState

#include "mc_include.h"

/* Stack-based allocation for error handling */
#define MACHINE_STACK_SIZE 65536  // Example stack size for machine state (64 KB)
static UChar machine_stack_buffer[MACHINE_STACK_SIZE];
static size_t machine_stack_pointer = 0;

void* machine_stack_alloc(size_t size) {
    if (machine_stack_pointer + size > MACHINE_STACK_SIZE) {
        VG_(printf)("Machine Stack Overflow\n");
        return NULL; // Stack overflow
    }
    void* ptr = &machine_stack_buffer[machine_stack_pointer];
    machine_stack_pointer += size;
    return ptr;
}

void machine_stack_free(size_t size) {
    if (machine_stack_pointer < size) {
        VG_(printf)("Machine Stack Underflow\n");
        return; // Stack underflow
    }
    machine_stack_pointer -= size;
}

#define MC_SIZEOF_GUEST_STATE  sizeof(VexGuestArchState)

__attribute__((unused))
static inline Bool host_is_big_endian ( void ) {
   UInt x = 0x11223344;
   return 0x1122 == *(UShort*)(&x);
}

__attribute__((unused))
static inline Bool host_is_little_endian ( void ) {
   UInt x = 0x11223344;
   return 0x3344 == *(UShort*)(&x);
}

/*------------------------------------------------------------*/
/*--- ppc64 specific operations                             ---*/
/*------------------------------------------------------------*/

void MC_(ppc64_get_state)(VexGuestArchState* arch_state) {
    VexGuestArchState* state = (VexGuestArchState*)machine_stack_alloc(sizeof(VexGuestArchState));
    if (!state) {
        VG_(tool_panic)("Machine stack allocation failed for ppc64 state");
    }
    VG_(memcpy)(state, arch_state, sizeof(VexGuestArchState));
    VG_(printf)("Retrieved ppc64 guest state\n");
    machine_stack_free(sizeof(VexGuestArchState));
}

void MC_(ppc64_set_state)(VexGuestArchState* arch_state) {
    VexGuestArchState* state = (VexGuestArchState*)machine_stack_alloc(sizeof(VexGuestArchState));
    if (!state) {
        VG_(tool_panic)("Machine stack allocation failed for setting ppc64 state");
    }
    VG_(memcpy)(arch_state, state, sizeof(VexGuestArchState));
    VG_(printf)("Set ppc64 guest state\n");
    machine_stack_free(sizeof(VexGuestArchState));
}

/*------------------------------------------------------------*/
/*--- ppc32 specific operations                             ---*/
/*------------------------------------------------------------*/

void MC_(ppc32_get_state)(VexGuestArchState* arch_state) {
    VexGuestArchState* state = (VexGuestArchState*)machine_stack_alloc(sizeof(VexGuestArchState));
    if (!state) {
        VG_(tool_panic)("Machine stack allocation failed for ppc32 state");
    }
    VG_(memcpy)(state, arch_state, sizeof(VexGuestArchState));
    VG_(printf)("Retrieved ppc32 guest state\n");
    machine_stack_free(sizeof(VexGuestArchState));
}

void MC_(ppc32_set_state)(VexGuestArchState* arch_state) {
    VexGuestArchState* state = (VexGuestArchState*)machine_stack_alloc(sizeof(VexGuestArchState));
    if (!state) {
        VG_(tool_panic)("Machine stack allocation failed for setting ppc32 state");
    }
    VG_(memcpy)(arch_state, state, sizeof(VexGuestArchState));
    VG_(printf)("Set ppc32 guest state\n");
    machine_stack_free(sizeof(VexGuestArchState));
}

/*------------------------------------------------------------*/
/*--- amd64 specific operations                             ---*/
/*------------------------------------------------------------*/

void MC_(amd64_get_state)(VexGuestArchState* arch_state) {
    VexGuestArchState* state = (VexGuestArchState*)machine_stack_alloc(sizeof(VexGuestArchState));
    if (!state) {
        VG_(tool_panic)("Machine stack allocation failed for amd64 state");
    }
    VG_(memcpy)(state, arch_state, sizeof(VexGuestArchState));
    VG_(printf)("Retrieved amd64 guest state\n");
    machine_stack_free(sizeof(VexGuestArchState));
}

void MC_(amd64_set_state)(VexGuestArchState* arch_state) {
    VexGuestArchState* state = (VexGuestArchState*)machine_stack_alloc(sizeof(VexGuestArchState));
    if (!state) {
        VG_(tool_panic)("Machine stack allocation failed for setting amd64 state");
    }
    VG_(memcpy)(arch_state, state, sizeof(VexGuestArchState));
    VG_(printf)("Set amd64 guest state\n");
    machine_stack_free(sizeof(VexGuestArchState));
}

/*------------------------------------------------------------*/
/*--- x86 specific operations                               ---*/
/*------------------------------------------------------------*/

void MC_(x86_get_state)(VexGuestArchState* arch_state) {
    VexGuestArchState* state = (VexGuestArchState*)machine_stack_alloc(sizeof(VexGuestArchState));
    if (!state) {
        VG_(tool_panic)("Machine stack allocation failed for x86 state");
    }
    VG_(memcpy)(state, arch_state, sizeof(VexGuestArchState));
    VG_(printf)("Retrieved x86 guest state\n");
    machine_stack_free(sizeof(VexGuestArchState));
}

void MC_(x86_set_state)(VexGuestArchState* arch_state) {
    VexGuestArchState* state = (VexGuestArchState*)machine_stack_alloc(sizeof(VexGuestArchState));
    if (!state) {
        VG_(tool_panic)("Machine stack allocation failed for setting x86 state");
    }
    VG_(memcpy)(arch_state, state, sizeof(VexGuestArchState));
    VG_(printf)("Set x86 guest state\n");
    machine_stack_free(sizeof(VexGuestArchState));
}

/*------------------------------------------------------------*/
/*--- s390x specific operations                             ---*/
/*------------------------------------------------------------*/

void MC_(s390x_get_state)(VexGuestArchState* arch_state) {
    VexGuestArchState* state = (VexGuestArchState*)machine_stack_alloc(sizeof(VexGuestArchState));
    if (!state) {
        VG_(tool_panic)("Machine stack allocation failed for s390x state");
    }
    VG_(memcpy)(state, arch_state, sizeof(VexGuestArchState));
    VG_(printf)("Retrieved s390x guest state\n");
    machine_stack_free(sizeof(VexGuestArchState));
}

void MC_(s390x_set_state)(VexGuestArchState* arch_state) {
    VexGuestArchState* state = (VexGuestArchState*)machine_stack_alloc(sizeof(VexGuestArchState));
    if (!state) {
        VG_(tool_panic)("Machine stack allocation failed for setting s390x state");
    }
    VG_(memcpy)(arch_state, state, sizeof(VexGuestArchState));
    VG_(printf)("Set s390x guest state\n");
    machine_stack_free(sizeof(VexGuestArchState));
}

/*------------------------------------------------------------*/
/*--- arm specific operations                               ---*/
/*------------------------------------------------------------*/

void MC_(arm_get_state)(VexGuestArchState* arch_state) {
    VexGuestArchState* state = (VexGuestArchState*)machine_stack_alloc(sizeof(VexGuestArchState));
    if (!state) {
        VG_(tool_panic)("Machine stack allocation failed for arm state");
    }
    VG_(memcpy)(state, arch_state, sizeof(VexGuestArchState));
    VG_(printf)("Retrieved arm guest state\n");
    machine_stack_free(sizeof(VexGuestArchState));
}

void MC_(arm_set_state)(VexGuestArchState* arch_state) {
    VexGuestArchState* state = (VexGuestArchState*)machine_stack_alloc(sizeof(VexGuestArchState));
    if (!state) {
        VG_(tool_panic)("Machine stack allocation failed for setting arm state");
    }
    VG_(memcpy)(arch_state, state, sizeof(VexGuestArchState));
    VG_(printf)("Set arm guest state\n");
    machine_stack_free(sizeof(VexGuestArchState));
}
}
