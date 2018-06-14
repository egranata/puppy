// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// from https://github.com/clemahieu/malloc/blob/master/malloc.c

#define NALLOC 1024

#include <kernel/sys/stdint.h>
#include <kernel/libc/string.h>
#include <kernel/libc/memory.h>
#include <kernel/mm/virt.h>

#define LOG_NODEBUG
#include <kernel/log/log.h>

#define sbrk(x) (char*)VirtualPageManager::get().ksbrk(x)

/* This header is stored at the beginning of memory segments in the list. */
union header {
  struct {
    union header *next;
    unsigned len;
  } meta;
  long x; /* Presence forces alignment of headers in memory. */
};

static union header list;
static union header *first = nullptr;

extern "C"
void free(void* ptr) {
  if (ptr == nullptr) {
    return;
  }

  union header *iter, *block;
  iter = first;
  block = (union header*)ptr - 1;

  /* Traverse to the spot in the list to insert the freed fragment,
   * such that the list is ordered by memory address (for coalescing). */
  while (block <= iter || block >= iter->meta.next) {
    if ((block > iter || block < iter->meta.next) &&
        iter >= iter->meta.next) {
      break;
    }
    iter = iter->meta.next;
  }

  /* If the new fragment is adjacent in memory to any others, merge
   * them (we only have to check the adjacent elements because the
   * order semantics are enforced). */
  if (block + block->meta.len == iter->meta.next) {
    block->meta.len += iter->meta.next->meta.len;
    block->meta.next = iter->meta.next->meta.next;
  } else {
    block->meta.next = iter->meta.next;
  }
  if (iter + iter->meta.len == block) {
    iter->meta.len += block->meta.len;
    iter->meta.next = block->meta.next;
  } else {
    iter->meta.next = block;
  }

  first = iter;
}

extern "C"
void *malloc(size_t size) {
  LOG_DEBUG("malloc(%u)", size);
  union header *p, *prev;
  prev = first;
  const unsigned true_size =
    (size + sizeof(union header) - 1) /
     sizeof(union header) + 1;

  LOG_DEBUG("true_size = %u", true_size);

  /* If the list of previously allocated fragments is empty,
   * initialize it. */
  if (first == nullptr) {
    LOG_DEBUG("first malloc call");
    prev = &list;
    first = prev;
    list.meta.next = first;
    list.meta.len = 0;
  }
  p = prev->meta.next;
  /* Traverse the list of previously allocated fragments, searching
   * for one sufficiently large to allocate. */
  while (1) {
    LOG_DEBUG("malloc found a block of len %u", p->meta.len);
    if (p->meta.len >= true_size) {
      if (p->meta.len == true_size) {
        /* If the fragment is exactly the right size, we do not have
         * to split it. */
        prev->meta.next = p->meta.next;
      } else {
        /* Otherwise, split the fragment, returning the first half and
         * storing the back half as another element in the list. */
        p->meta.len -= true_size;
        LOG_DEBUG("malloc left a block of len %u", p->meta.len);
        p += p->meta.len;
        p->meta.len = true_size;
      }
      first = prev;
      LOG_DEBUG("returning %p", p+1);
      return (void *)(p+1);
    }
    /* If we reach the beginning of the list, no satisfactory fragment
     * was found, so we have to request a new one. */
    if (p == first) {
      char *page;
      union header *block;
      /* We have to request memory of at least a certain size. */
	  auto sbrk_size = (true_size >= NALLOC ? true_size : NALLOC);
    auto kernel_ask_size = (uintptr_t) (sbrk_size * sizeof(union header));
    LOG_DEBUG("asking the kernel for %u bytes", kernel_ask_size);
      page = sbrk(kernel_ask_size);
      LOG_DEBUG("kernel said %p", page);
      if (page == (char *)-1) {
      LOG_DEBUG("returning nullptr");
		  return nullptr;
      }
      /* Create a fragment from this new memory and add it to the list
       * so the above logic can handle breaking it if necessary. */
      block = (union header *)page;
      block->meta.len = sbrk_size;
      free((void *)(block + 1));
      p = first;
    }

    prev = p;
    p = p->meta.next;
  }
  LOG_DEBUG("returning nullptr");
  return nullptr;
}

extern "C"
void* calloc(size_t num, size_t len) {
  void* ptr = malloc(num * len);

  /* Set the allocated array to 0's.*/
  if (ptr != nullptr) {
    bzero((uint8_t*)ptr, num * len);
  }

  return ptr;
}

extern "C"
void* realloc(void *ptr, size_t new_size) {
  size_t old_size = 0; /* XXX yolo */
  void* newp = malloc(new_size);

  if (newp != nullptr) {
    /* We cannot grow the memory segment. */
    if (new_size > old_size) {
      new_size = old_size;
    }
    memcopy((uint8_t*)ptr, (uint8_t*)newp, new_size);
    free(ptr);
  }

  return newp;
}