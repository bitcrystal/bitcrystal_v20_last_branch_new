#include <stdio.h>
#include <stdlib.h>

#include "vector_c.h"

void vector_init_c(vector_c *v)
{
    v->capacity = VECTOR_INIT_CAPACITY_C;
    v->total = 0;
    v->items = malloc(sizeof(void *) * v->capacity);
}

int vector_total_c(vector_c *v)
{
    return v->total;
}

static void vector_resize_c(vector_c *v, int capacity)
{
    #ifdef DEBUG_ON
    printf("vector_resize: %d to %d\n", v->capacity, capacity);
    #endif

    void **items = realloc(v->items, sizeof(void *) * capacity);
    if (items) {
        v->items = items;
        v->capacity = capacity;
    }
}

void vector_add_c(vector_c *v, void *item)
{
    if (v->capacity == v->total)
        vector_resize_c(v, v->capacity * 2);
    v->items[v->total++] = item;
}

void vector_set_c(vector_c *v, int index, void *item)
{
    if (index >= 0 && index < v->total)
        v->items[index] = item;
}

void *vector_get_c(vector_c *v, int index)
{
    if (index >= 0 && index < v->total)
        return v->items[index];
    return NULL;
}

void vector_delete_c(vector_c *v, int index)
{
    if (index < 0 || index >= v->total)
        return;

    v->items[index] = NULL;

    for (int i = 0; i < v->total - 1; i++) {
        v->items[i] = v->items[i + 1];
        v->items[i + 1] = NULL;
    }

    v->total--;

    if (v->total > 0 && v->total == v->capacity / 4)
        vector_resize_c(v, v->capacity / 2);
}

void vector_free_c(vector_c *v)
{
    free(v->items);
}