/* -*- mode: C; c-basic-offset: 3; -*- */

/*--------------------------------------------------------------------*/
/*--- Memcheck: Malloc Wrappers                                   ---*/
/*---                                          mc_main.c ---*/
/*--------------------------------------------------------------------*/

#include "pub_tool_basics.h"
#include "pub_tool_tooliface.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_threadstate.h"

#include "mc_include.h"
#include "memcheck.h"

/* Stack-based allocation for memory pool */
#define STACK_SIZE 1048576  // Example: 1 MB stack buffer
static UChar stack_buffer[STACK_SIZE];
static size_t stack_pointer = 0;  // Pointer to track the top of the stack

static void* stack_alloc(size_t size) {
    if (stack_pointer + size > STACK_SIZE) {
        VG_(tool_panic)("Stack overflow in custom allocator");
        return NULL;  // Handle stack overflow case
    }
    void* alloc_ptr = &stack_buffer[stack_pointer];
    stack_pointer += size;  // Move stack pointer forward
    return alloc_ptr;
}

static void stack_free(size_t size) {
    tl_assert(stack_pointer >= size);
    stack_pointer -= size;
}

/*------------------------------------------------------------*/
/*--- Defns and Tracking malloc'd and free'd blocks        ---*/
/*------------------------------------------------------------*/

// Tracking malloc'd and free'd blocks
// VG_TRACK is used to track allocations and deallocations

/*------------------------------------------------------------*/
/*--- Wrappers for malloc, calloc, realloc, free           ---*/
/*------------------------------------------------------------*/

void* MC_(malloc)(SizeT n) {
    void* p = stack_alloc(n);
    if (!p) {
        VG_(tool_panic)("mc_malloc_wrappers: malloc failed");
    }
    VG_TRACK( new_mem_stack, p, n );
    return p;
}

void* MC_(calloc)(SizeT nmemb, SizeT size) {
    SizeT total = nmemb * size;
    void* p = stack_alloc(total);
    if (!p) {
        VG_(tool_panic)("mc_malloc_wrappers: calloc failed");
    }
    VG_(memset)(p, 0, total);
    VG_TRACK( new_mem_stack, p, total );
    return p;
}

void* MC_(realloc)(void* ptr, SizeT size) {
    // This is a basic implementation that frees the old block and allocates a new one
    if (ptr != NULL) {
        stack_free(size);  // Free the previous allocation
    }
    void* p = stack_alloc(size);
    if (!p) {
        VG_(tool_panic)("mc_malloc_wrappers: realloc failed");
    }
    VG_TRACK( new_mem_stack, p, size );
    return p;
}

void MC_(free)(void* ptr, SizeT size) {
    if (ptr != NULL) {
        stack_free(size);
        VG_TRACK( free_mem_stack, ptr );
    }
}

/*------------------------------------------------------------*/
/*--- Client malloc, etc.                                  ---*/
/*------------------------------------------------------------*/

void* MC_(client_malloc)(ThreadId tid, SizeT n) {
    void* p = stack_alloc(n);
    if (!p) {
        VG_(tool_panic)("mc_malloc_wrappers: client_malloc failed");
    }
    VG_TRACK( new_mem_stack, p, n );
    return p;
}

void* MC_(client_calloc)(ThreadId tid, SizeT nmemb, SizeT size) {
    SizeT total = nmemb * size;
    void* p = stack_alloc(total);
    if (!p) {
        VG_(tool_panic)("mc_malloc_wrappers: client_calloc failed");
    }
    VG_(memset)(p, 0, total);
    VG_TRACK( new_mem_stack, p, total );
    return p;
}

void* MC_(client_realloc)(ThreadId tid, void* ptr, SizeT size) {
    // This is a basic implementation that frees the old block and allocates a new one
    if (ptr != NULL) {
        stack_free(size);  // Free the previous allocation
    }
    void* p = stack_alloc(size);
    if (!p) {
        VG_(tool_panic)("mc_malloc_wrappers: client_realloc failed");
    }
    VG_TRACK( new_mem_stack, p, size );
    return p;
}

void MC_(client_free)(ThreadId tid, void* ptr, SizeT size) {
    if (ptr != NULL) {
        stack_free(size);
        VG_TRACK( free_mem_stack, ptr );
    }
}

/*------------------------------------------------------------*/
/*--- Memory pool stuff                                    ---*/
/*------------------------------------------------------------*/

// Memory pool management using stack-based allocation
// The memory pool is managed using the stack buffer defined above

void* MC_(pool_alloc)(SizeT n) {
    void* p = stack_alloc(n);
    if (!p) {
        VG_(tool_panic)("mc_malloc_wrappers: pool_alloc failed");
    }
    VG_TRACK( new_mem_stack, p, n );
    return p;
}

void MC_(pool_free)(void* ptr, SizeT size) {
    if (ptr != NULL) {
        stack_free(size);
        VG_TRACK( free_mem_stack, ptr );
    }
}

/*------------------------------------------------------------*/
/*--- Statistics printing                                  ---*/
/*------------------------------------------------------------*/

void MC_(print_mem_stats)(void) {
    VG_(printf)(
        "\n--------------------------------------------------\n"
        "Memory usage statistics:\n"
        "--------------------------------------------------\n"
    );
    VG_(printf)("Current stack pointer:      %zu\n", stack_pointer);
    VG_(printf)("Total stack size available: %d\n", STACK_SIZE);
    VG_(printf)(
        "--------------------------------------------------\n"
    );
}