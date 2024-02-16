// SPDX-License-Identifier: BSD-3-Clause

#include <internal/mm/mem_list.h>
#include <internal/types.h>
#include <internal/essentials.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// used "musl" as reference(https://www.musl-libc.org/)
void *malloc(size_t size)
{
	// se mmap to allocate memory
	void *mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (mem == MAP_FAILED) {
		// allocation failed
		return NULL;
	}
	mem_list_add(mem, size);

	return mem;
}

void *calloc(size_t nmemb, size_t size)
{
	// calculate the total memory size
	size_t total_size = nmemb * size;

	// allocate memory
	void *mem = malloc(total_size);

	if (mem)
		memset(mem, 0, total_size);

	return mem;
}

void free(void *ptr)
{
	/* TODO: Implement free(). */
	struct mem_list *nod = mem_list_find(ptr);

	munmap(ptr, nod->len);
	mem_list_del(ptr);
}

void *realloc(void *ptr, size_t size) {
	void *new_ptr = mmap(NULL, size, PROT_READ | PROT_WRITE,
					   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (new_ptr == MAP_FAILED)
		return NULL;
	// copy old allocated space to new one
	memcpy(new_ptr, ptr, size);
	free(ptr);
	return new_ptr;
}

void *reallocarray(void *ptr, size_t nmemb, size_t size) {
  void *new_ptr = mmap(NULL, nmemb * size, PROT_READ | PROT_WRITE,
					   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (new_ptr == MAP_FAILED)
	return NULL;

	memcpy(new_ptr, ptr, nmemb * size);
	free(ptr);

	return new_ptr;
}
