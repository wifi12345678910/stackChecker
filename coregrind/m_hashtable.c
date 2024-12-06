
/*--------------------------------------------------------------------*/
/*--- A separately-chained hash table.               m_hashtable.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Valgrind, a dynamic binary instrumentation
   framework.

   Copyright (C) 2005-2017 Nicholas Nethercote
      njn@valgrind.org

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
*/

#include "pub_core_basics.h"
#include "pub_core_debuglog.h"
#include "pub_core_hashtable.h"
#include "pub_core_libcassert.h"
#include "pub_core_libcbase.h"
#include "pub_core_libcprint.h"
#include "pub_core_mallocfree.h"

/*--------------------------------------------------------------------*/
/*--- Declarations                                                 ---*/
/*--------------------------------------------------------------------*/

#define CHAIN_NO(key,tbl) (((ULong)(key)) % tbl->max_size)

struct _VgHashTable {
   SizeT        max_size;   // should be prime
   UInt         n_elements;
   VgHashNode*  iterNode;   // current iterator node
   UInt         iterChain;  // next chain to be traversed by the iterator
   Bool         iterOK;     // table safe to iterate over?
   VgHashNode** chain;     // expanding array of hash chains
   const HChar* name;       // name of table (for debugging only)
};

#define N_HASH_PRIMES 20

static const SizeT primes[N_HASH_PRIMES] = {
         769UL,         1543UL,         3079UL,          6151UL,
       12289UL,        24593UL,        49157UL,         98317UL,
      196613UL,       393241UL,       786433UL,       1572869UL,
     3145739UL,      6291469UL,     12582917UL,      25165843UL,
    50331653UL,    100663319UL,    201326611UL,     402653189UL
};

/*--------------------------------------------------------------------*/
/*--- Functions                                                    ---*/
/*--------------------------------------------------------------------*/

VgHashTable *VG_(HT_construct) ( const HChar* name )
{
   /* Initialises to zero, ie. all entries NULL */
   SizeT       max_size = primes[0];
   SizeT       sz       = max_size * sizeof(VgHashNode*);
   VgHashTable *table   = VG_(calloc)("hashtable.Hc.1",
                                      1, sizeof(struct _VgHashTable));
   table->chain        = VG_(calloc)("hashtable.Hc.2", 1, sz);
   //table->n_chains      = n_chains;
   table->n_elements    = 0;
   table->iterOK        = True;
   table->name          = name;
   table->max_size = max_size;
   vg_assert(name);
   return table;
}

UInt VG_(HT_count_nodes) ( const VgHashTable *table )
{
   return table->n_elements;
}

static void resize ( VgHashTable *table )
{
   Int          i;
   SizeT        sz;
   SizeT        old_chains = table->max_size;
   SizeT        new_chains = old_chains + 1;
   VgHashNode** chains;
   VgHashNode * node;

   /* If we've run out of primes, do nothing. */
   if (old_chains == primes[N_HASH_PRIMES-1])
      return;

   vg_assert(old_chains >= primes[0] 
             && old_chains < primes[N_HASH_PRIMES-1]);

   for (i = 0; i < N_HASH_PRIMES; i++) {
      if (primes[i] > new_chains) {
         new_chains = primes[i];
         break;
      }
   }

   vg_assert(new_chains > old_chains);
   vg_assert(new_chains > primes[0] 
             && new_chains <= primes[N_HASH_PRIMES-1]);

   VG_(debugLog)(
      1, "hashtable",
         "resizing table `%s' from %lu to %lu (total elems %lu)\n",
         table->name, (UWord)old_chains, (UWord)new_chains,
         (UWord)table->n_elements );

   table->max_size = new_chains;
   sz = new_chains * sizeof(VgHashNode*);
   chains = VG_(calloc)("hashtable.resize.1", 1, sz);

   for (i = 0; i < old_chains; i++) {
      node = table->chain[i];
    //     printf("131");
      if(node != NULL) {
         ULong idx = CHAIN_NO(node->key,table);
         while (chains[idx] == NULL) {idx++;}
            chains[idx] = node;
      }
   }

   VG_(free)(table->chain);
   table->chain = chains;
}

/* Puts a new, heap allocated VgHashNode, into the VgHashTable.  Prepends
   the node to the appropriate chain.  No duplicate key detection is done. */
void VG_(HT_add_node) ( VgHashTable *table, void* vnode )
{
   VgHashNode* node     = (VgHashNode*)vnode;
//   printf("154");
   UWord idx          = CHAIN_NO(node->key, table);
   while (table->chain[idx])
   {
      idx++;

    if (idx > table->max_size-1) {

         resize(table);
      }
   }
   table->chain[idx] = node;
   table->chain[idx]->next = NULL;
   
   table->n_elements++;

   /* Table has been modified; hence HT_Next should assert. */
   table->iterOK = False;
}

/* Looks up a VgHashNode by key in the table.  Returns NULL if not found. */
void* VG_(HT_lookup) ( const VgHashTable *table, UWord key )
{
  // printf("177");
  ULong idx = CHAIN_NO(key, table) ;
   VgHashNode* curr = table->chain[ idx];

   while (curr) {
      if (key == curr->key) {
         return curr;
      }
      idx++;
      if (idx >= table->max_size){break;}
      curr = table->chain[idx];
   }
   return NULL;
}

/* Looks up a VgHashNode by node in the table.  Returns NULL if not found.
   GEN!!! marks the lines that differs from VG_(HT_lookup). */
void* VG_(HT_gen_lookup) ( const VgHashTable *table, const void* node,
                           HT_Cmp_t cmp )
{
   const VgHashNode* hnode = node; // GEN!!!
 //  printf("195");
   if (table->max_size == 0) {
      return NULL;
   }
   ULong idx = CHAIN_NO(hnode->key, table);
   VgHashNode* curr = table->chain[ idx ]; // GEN!!!

   while (curr) {
      if (hnode->key == curr->key && cmp (hnode, curr) == 0) { // GEN!!!
         return curr;
      }
      idx++;
      if (idx >= table->max_size){break;}
      curr = table->chain[idx];
   }
   return NULL;
}

/* Removes a VgHashNode from the table.  Returns NULL if not found. */
void* VG_(HT_remove) ( VgHashTable *table, UWord key )
{
  // printf("210");
   UWord        idx         = CHAIN_NO(key, table);
   VgHashNode*  curr          =   table->chain[idx];
   //VgHashNode** prev_next_ptr = &(table->chain[idx]);

   /* Table has been modified; hence HT_Next should assert. */
   table->iterOK = False;

   for (UWord j = 0; j < table->n_elements; j++){
      if (key == curr->key) {
      //   *prev_next_ptr = curr->next;
         table->n_elements--;
         return curr;
      }
      for (UWord i = 0; i+j < table->max_size; i++) {
         if (table->chain[i] != NULL){
            curr = table->chain[i];
      //      prev_next_ptr = &(curr);
            break;
         } else {
               curr = NULL;
         }
      }
   }
   return NULL;
}

/* Removes a VgHashNode by node from the table.  Returns NULL if not found.
   GEN!!! marks the lines that differs from VG_(HT_remove). */
void* VG_(HT_gen_remove) ( VgHashTable *table, const void* node, HT_Cmp_t cmp  )
{
   const VgHashNode* hnode    = node; // GEN!!!
  // printf("235");
   UWord        idx         = CHAIN_NO(hnode->key, table); // GEN!!!
   VgHashNode*  curr          =   table->chain[idx];
  //   VgHashNode** prev_next_ptr = &(table->chain[idx]);

   /* Table has been modified; hence HT_Next should assert. */
   table->iterOK = False;

   while (curr) {
      if (hnode->key == curr->key && cmp(hnode, curr) == 0) { // GEN!!!
  //       *prev_next_ptr = curr->next;
         table->n_elements--;
         return curr;
      }
      for (UWord i = 0; i +idx < table->max_size; i++) {
            if (table->chain[i] != NULL){
               curr = table->chain[i];
            //   prev_next_ptr = &(curr);
               break;
            } else {
            curr = NULL;
         }
      }
   }
   return NULL;
}

void VG_(HT_print_stats) ( const VgHashTable *table, HT_Cmp_t cmp )
{
 /*  #define MAXOCCUR 20
   UInt elt_occurences[MAXOCCUR+1];
   UInt key_occurences[MAXOCCUR+1];
   UInt cno_occurences[MAXOCCUR+1];
    Key occurence  : how many ht elements have the same key.
      elt_occurences : how many elements are inserted multiple time.
      cno_occurences : how many chains have that length.
      The last entry in these arrays collects all occurences >= MAXOCCUR. 
   #define INCOCCUR(occur,n) (n >= MAXOCCUR ? occur[MAXOCCUR]++ : occur[n]++)
   UInt i;
   UInt nkey, nelt, ncno;
   VgHashNode *cnode, *node;

   VG_(memset)(key_occurences, 0, sizeof(key_occurences));
   VG_(memset)(elt_occurences, 0, sizeof(elt_occurences));
   VG_(memset)(cno_occurences, 0, sizeof(cno_occurences));

   // Note that the below algorithm is quadractic in nr of elements in a chain
   // but if that happens, the hash table/function is really bad and that
   // should be fixed.
   for (i = 0; i < table->n_chains; i++) {
      ncno = 0;
      for (cnode = table->chains[i]; cnode != NULL; cnode = cnode->next) {
         ncno++;

         nkey = 0;
         // Is the same cnode->key existing before cnode ?
         for (node = table->chains[i]; node != cnode; node = node->next) {
            if (node->key == cnode->key)
               nkey++;
         }
         // If cnode->key not in a previous node, count occurences of key.
         if (nkey == 0) {
            for (node = cnode; node != NULL; node = node->next) {
               if (node->key == cnode->key)
                  nkey++;
            }
            INCOCCUR(key_occurences, nkey);
         }

         nelt = 0;
         // Is the same cnode element existing before cnode ?
         for (node = table->chains[i]; node != cnode; node = node->next) {
            if (node->key == cnode->key
                && (cmp == NULL || cmp (node, cnode) == 0)) {
               nelt++;
            }
         }
         // If cnode element not in a previous node, count occurences of elt.
         if (nelt == 0) {
            for (node = cnode; node != NULL; node = node->next) {
               if (node->key == cnode->key
                   && (cmp == NULL || cmp (node, cnode) == 0)) {
                  nelt++;
               }
            }
            INCOCCUR(elt_occurences, nelt);
         }
      }
      INCOCCUR(cno_occurences, ncno);
   }

   VG_(message)(Vg_DebugMsg, 
                "nr occurrences of"
                " chains of len N,"
                " N-plicated keys,"
                " N-plicated elts\n");
   nkey = nelt = ncno = 0;
   for (i = 0; i <= MAXOCCUR; i++) {
      if (elt_occurences[i] > 0 
          || key_occurences[i] > 0 
          || cno_occurences[i] > 0)
         VG_(message)(Vg_DebugMsg,
                      "%s=%2u : nr chain %6u, nr keys %6u, nr elts %6u\n",
                      i == MAXOCCUR ? ">" : "N", i,
                      cno_occurences[i], key_occurences[i], elt_occurences[i]);
      nkey += key_occurences[i];
      nelt += elt_occurences[i];
      ncno += cno_occurences[i];
   }
   VG_(message)(Vg_DebugMsg, 
                "total nr of unique   slots: %6u, keys %6u, elts %6u."
                " Avg chain len %3.1f\n",
                ncno, nkey, nelt,
                (Double)nelt/(Double)(ncno == cno_occurences[0] ?
                                      1 : ncno - cno_occurences[0]));
   */
}


/* Allocates a suitably-sized array, copies pointers to all the hashtable
   elements into it, then returns both the array and the size of it.  The
   array must be freed with VG_(free).
*/
VgHashNode** VG_(HT_to_array) (const VgHashTable *table, /*OUT*/ UInt* n_elems)
{
   UInt        j;
   VgHashNode** arr;
   VgHashNode*  node;

   *n_elems = table->n_elements;
   if (*n_elems == 0)
      return NULL;

   arr = VG_(malloc)( "hashtable.Hta.1", *n_elems * sizeof(VgHashNode*) );

   j = 0;
   for (UInt i = 0; i<table->max_size; i++) {
      node = table->chain[i];
      while (node){
         arr[j++] = node;
         node = node->next;
         i++;
      }
   }
   vg_assert(j == *n_elems);

   return arr;
}

void VG_(HT_ResetIter)(VgHashTable *table)
{
   vg_assert(table);
   table->iterNode  = NULL;
   table->iterChain = 0;
   table->iterOK    = True;
}

void* VG_(HT_Next)(VgHashTable *table)
{
   Int i;
   vg_assert(table);
   /* See long comment on HT_Next prototype in pub_tool_hashtable.h.
      In short if this fails, it means the caller tried to modify the
      table whilst iterating over it, which is a bug.
      One exception: HT_remove_at_Iter can remove the current entry and
      leave the iterator in a valid state for HT_Next. */
   vg_assert(table->iterOK);

   if (table->iterNode && table->iterNode->next) {
      table->iterNode = table->iterNode->next;
      table->iterChain++;
      return table->iterNode;
   }

   for (i = table->iterChain; i < table->max_size; i++) {
      if (table->chain[i] != NULL) {
         table->iterNode  = table->chain[i];
         table->iterChain = i + 1;  // Next chain to be traversed
         return table->iterNode;
      }
   }
   return NULL;
}

void VG_(HT_remove_at_Iter)(VgHashTable *table)
{
   vg_assert(table);
   vg_assert(table->iterOK);
   vg_assert(table->iterNode);

   const UInt curChain = table->iterChain - 1; // chain of iterNode.
   
   table->iterNode = NULL;
   table->n_elements--;
}

void VG_(HT_destruct)(VgHashTable *table, void(*freenode_fn)(void*))
{
   UInt       i;
   VgHashNode *node, *node_next;

   for (i = 0; i < table->max_size; i++) {
      node =  table->chain[i];
      if (node != NULL) {
         freenode_fn(node);
      }
   }
   VG_(free)(table->chain);
   VG_(free)(table);
}

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/
