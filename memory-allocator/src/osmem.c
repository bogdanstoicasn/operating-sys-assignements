// SPDX-License-Identifier: BSD-3-Clause

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include "printf.h"
#include "osmem.h"
#include "block_meta.h"
#define MMAP_THRESHOLD (128 * 1024)
#define N_ALIGN_N 8
#define ALIGN_8BYTE(size) ((size + 7) & (~7))
#define BLOCK_SIZE sizeof(struct block_meta)
#define MAP_ANONYMOUS 0x20
#define PAGE_SIZE 4080

struct block_meta *global_base;
struct block_meta *large_alloc;

// init heap
void init_heap(void)
{
	if (!global_base) {
		global_base = sbrk(0);
		void *heap_end = sbrk(MMAP_THRESHOLD);

		if (heap_end == (void *)-1)
			return;
		struct block_meta *first_block = global_base;

		first_block->size = MMAP_THRESHOLD - BLOCK_SIZE;
		first_block->status = STATUS_FREE;
		first_block->next = NULL;
		first_block->prev = NULL;
	}
}

// obtain a free block
struct block_meta *get_free_block(size_t size)
{
	struct block_meta *current = global_base;

	while (current) {
		if (current->status == STATUS_FREE && current->size >= size)
			return current;
		current = current->next;
	}
	return NULL;
}

// obtain the last block from the sbrk list
struct block_meta *get_last_block(void)
{
	if (!global_base)
		return NULL;

	struct block_meta *current = global_base;

	while (current->next)
		current = current->next;


	return current;
}

// expand the heap
void expand_heap(size_t size)
{
	struct block_meta *last = global_base;

	while (last && last->next)
		last = last->next;

	if (last->status == STATUS_FREE && last->size < size) {
		// expand is last block free but little size
		size_t total_size = size - last->size;

		void *heap_end = sbrk(total_size);

		if (heap_end == (void *)-1)
			return;

		last->size += total_size;
	} else {
		size_t total_size = size + BLOCK_SIZE;

		void *heap = sbrk(0);

		if (heap == (void *)-1)
			return;

		void *heap_end = sbrk(total_size);

		if (heap_end == (void *)-1)
			return;

		struct block_meta *new_block = heap;

		new_block->size = size;
		new_block->status = STATUS_FREE;
		new_block->prev = last;
		new_block->next = NULL;

		if (last)
			last->next = new_block;
		else
			global_base = new_block;
	}
}

// coalesce free blocks from the sbrk list
void coalesce_blocks(void)
{
	struct block_meta *current = global_base;

	while (current) {
		if (current->status == STATUS_FREE && current->next && current->next->status == STATUS_FREE) {
			current->size += current->next->size + BLOCK_SIZE;
			current->next = current->next->next;
			if (current->next)
				current->next->prev = current;
		}
		current = current->next;
	}
}

// split a block
void split_block(struct block_meta *block, size_t size)
{
	if (block->size >= size) {
		size_t remaining_space = block->size - size;
		size_t total_block_size = size + BLOCK_SIZE;

		if (remaining_space > BLOCK_SIZE) {
			struct block_meta *new_block = (struct block_meta *)((char *)block + total_block_size);

			new_block->size = remaining_space - BLOCK_SIZE;
			new_block->status = STATUS_FREE;
			new_block->prev = block;
			new_block->next = block->next;

			block->size = size;
			block->next = new_block;

			if (new_block->next)
				new_block->next->prev = new_block;
		}
	}
}

// search a block from the sbrk list
void *search_block_sbrk(struct block_meta *block)
{
	if (!block)
		return NULL;

	struct block_meta *current = global_base;

	while (current) {
		if (current == block)
			return current;
		current = current->next;
	}
	return NULL;
}

// coalesce free blocks from the mmap list
void coalesce_mmap(void)
{
	struct block_meta *current = large_alloc;

	while (current) {
		if (current->status == STATUS_FREE && current->next && current->next->status == STATUS_FREE) {
			current->size += current->next->size + BLOCK_SIZE;
			current->next = current->next->next;
			if (current->next)
				current->next->prev = current;
		}
		current = current->next;
	}
}

void *os_malloc(size_t size)
{
	/* TODO: Implement os_malloc */
	size_t new_size = ALIGN_8BYTE(size);

	if (size == 0)
		return NULL;

	if (new_size < MMAP_THRESHOLD - BLOCK_SIZE) {
		init_heap();

		struct block_meta *best = get_free_block(new_size);

		if (best) {
			best->status = STATUS_ALLOC;
			split_block(best, new_size);
			coalesce_blocks();
			return (void *)(best + 1);
		}

		expand_heap(new_size);

		best = get_free_block(new_size);
		if (best) {
			best->status = STATUS_ALLOC;
			split_block(best, new_size);
			coalesce_blocks();
			return (void *)(best + 1);
		}
	}
	// mmap allocation
	void *block = mmap(NULL, new_size + BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (block == MAP_FAILED)
		return NULL;

	struct block_meta *new_block = block;

	new_block->size = new_size;
	new_block->status = STATUS_MAPPED;
	new_block->prev = NULL;
	new_block->next = large_alloc;
	if (large_alloc)
		large_alloc->prev = new_block;

	large_alloc = new_block;
	coalesce_mmap();
	return (void *)(new_block + 1);
}

// free a block
void os_free(void *ptr)
{
	if (!ptr)
		return; // NULL

	struct block_meta *block = ptr - 32; // Point to the metadata

	if (block->status == STATUS_FREE) {
		(void)ptr;
		return;
	}

	struct block_meta *last_block = get_last_block();

	if (block >= global_base && block <= last_block) {
		// the block is from heap allocation
		block->status = STATUS_FREE;
		coalesce_blocks();
		return;
	}

	// the block is from mmap allocation
	if (block == large_alloc)
		large_alloc = block->next;
	if (block->prev)
		block->prev->next = block->next;
	if (block->next)
		block->next->prev = block->prev;

	block->status = STATUS_FREE;
	munmap(block, block->size + BLOCK_SIZE);
}


void *os_calloc(size_t nmemb, size_t size)
{
	/* TODO: Implement os_calloc */
	size_t cc = size * nmemb;
	size_t new_size = ALIGN_8BYTE(cc);

	if (nmemb == 0 || size == 0)
		return NULL;

	if ((int)new_size < PAGE_SIZE) {
		void *ptr = os_malloc(new_size);

		if (ptr)
			memset(ptr, 0, new_size);
		return ptr;
	}
	void *block = mmap(NULL, new_size + BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (block == MAP_FAILED)
		return NULL;

	struct block_meta *new_block = block;

	new_block->size = new_size;
	new_block->status = STATUS_MAPPED;
	new_block->prev = NULL;
	new_block->next = large_alloc;
	if (large_alloc)
		large_alloc->prev = new_block;
	large_alloc = new_block;
	memset(block + 32, 0, new_size);
	return (void *)(new_block + 1);
}

// remove a block from the mmap list
void remove_from_mmap_list(struct block_meta *block)
{
	if (!block || block->status != STATUS_MAPPED)
		return;

	if (block->prev)
		block->prev->next = block->next;
	else
		large_alloc = block->next;

	if (block->next)
		block->next->prev = block->prev;

	block->prev = NULL;
	block->next = NULL;
	block->status = STATUS_FREE;  // Assuming it's now a free block
}

void *os_realloc(void *ptr, size_t size)
{
	if (!size) {
		os_free(ptr);
		ptr = NULL;
		return NULL;
	}

	if (!ptr)
		return os_malloc(size);

	if (!size && !ptr)
		return NULL;

	size_t new_size = ALIGN_8BYTE(size);
	struct block_meta *block = ptr - 32; // Point to the metadata

	if (new_size < MMAP_THRESHOLD - BLOCK_SIZE) {
		struct block_meta *verify = search_block_sbrk(block);

		if (!verify) {
			// we must remove the block from the mmap list
			remove_from_mmap_list(block);
			// and add it to the sbrk list
			struct block_meta *new_block = get_free_block(new_size);

			if (new_block) {
				new_block->status = STATUS_ALLOC;
				split_block(new_block, new_size);
				coalesce_blocks();
				memcpy(new_block, ptr, block->size);
				return (void *)(new_block + 1);
			}

			// if we can't find a free block, we need to expand the heap
			expand_heap(new_size);
			new_block = get_free_block(new_size);
			if (new_block) {
				new_block->status = STATUS_ALLOC;
				split_block(new_block, new_size);
				coalesce_blocks();
				memcpy(new_block, ptr, block->size);
				return (void *)(new_block + 1);
			}
			return NULL;
		}

		// the block is from heap allocation
		if (block->size >= new_size) {
			split_block(block, new_size);
			coalesce_blocks();
			return ptr;
		}
			struct block_meta *next = block->next;

			if (next && next->status == STATUS_FREE && block->size + next->size + BLOCK_SIZE >= new_size) {
				block->size += next->size + BLOCK_SIZE;
				block->next = next->next;
				if (next->next)
					next->next->prev = block;
				split_block(block, new_size);
				coalesce_blocks();
				return ptr;
			}
			void *new_block = os_malloc(new_size);

			if (!new_block)
				return NULL;
			memcpy(new_block, ptr, block->size);
			os_free(ptr);
			coalesce_blocks();
			return new_block;
	}

	// the block is from mmap allocation
	struct block_meta *verify = search_block_sbrk(block);

	if (verify) {
		// we must remove the block from the sbrk list
		if (block->prev)
			block->prev->next = block->next;
		else
			global_base = block->next;

		if (block->next)
			block->next->prev = block->prev;

		block->status = STATUS_FREE;  // assuming it's now a free block
		// we must update the size of tprevious block
		if (block->prev) {
			block->prev->size += block->size + BLOCK_SIZE;
			block = block->prev;
		}
		// and add it to the mmap list
		void *new_block = os_malloc(new_size);

		if (!new_block)
			return NULL;
		memcpy(new_block, ptr, block->size);
		os_free(ptr);
		coalesce_blocks();
		return new_block;
	}
	// the block is from mmap and remains on mmap
	if (block->size >= new_size) {
		split_block(block, new_size);
		return ptr;
	}

	void *new_block = os_malloc(new_size);

	if (!new_block)
		return NULL;
	memcpy(new_block, ptr, block->size);
	os_free(ptr);
	return new_block;
}

