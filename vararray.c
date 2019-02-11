#include "vararray.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

struct vararray_t
{
	unsigned int capacity;
	unsigned int size;
	void ** data;
};

vararray_handle vararray_create()
{
	vararray_t * ret = malloc(sizeof(vararray_t));
	ret->capacity = INITIAL_CAPACITY;
	ret->size  = 0;
	ret->data = (void **)malloc(sizeof(void *)*INITIAL_CAPACITY);
	return ret;
}

// The vararray won't be responsible for freeing any elements
// so the only thing we have to do is tell the array that
// it has 0 elements
void vararray_clear(vararray_handle h)
{
	h->size = 0;
}

unsigned int vararray_size(vararray_handle h)
{
	return h->size;
}

void vararray_push_back(vararray_handle h, void * element)
{
	assert(h->size<=h->capacity);
	if (h->size==h->capacity) // double the capacity
	{
		void ** copy = malloc(sizeof(void *)*(h->capacity*2));
		memcpy(copy, h->data, h->capacity * sizeof(void *));
		free(h->data);
		h->data = copy;
		h->capacity = h->capacity * 2;
	}
	// push element in
	h->data[h->size] = element;
	h->size++;
}

void * vararray_get(vararray_handle h, unsigned int idx)
{
	assert(idx < h->size);
	return h->data[idx];
}

void vararray_destroy(vararray_handle h)
{
	free(h->data);
	free(h);
}
