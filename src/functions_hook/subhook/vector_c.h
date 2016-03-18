#ifndef VECTOR_C_H
#define VECTOR_C_H

#define VECTOR_INIT_C_CAPACITY 4

#define VECTOR_INIT_C(vec) vector_c vec; vector_c_init(&vec)
#define VECTOR_INIT_C_EX(vec) vector_c_init(vec)
#define VECTOR_ADD_C(vec, item) vector_add_c(&vec, (void *) item)
#define VECTOR_ADD_C_EX(vec, item) vector_add_c(vec, (void *) item)
#define VECTOR_SET_C(vec, id, item) vector_set_c(&vec, id, (void *) item)
#define VECTOR_SET_C_EX(vec, id, item) vector_set_c(vec, id, (void *) item)
#define VECTOR_GET_C(vec, type, id) (type) vector_get_c(&vec, id)
#define VECTOR_GET_C_EX(vec, type, id) (type) vector_get_c(vec, id)
#define VECTOR_DELETE_C(vec, id) vector_delete_c(&vec, id)
#define VECTOR_DELETE_C_EX(vec, id) vector_delete_c(vec, id)
#define VECTOR_TOTAL_C(vec) vector_total_c(&vec)
#define VECTOR_TOTAL_C_EX(vec) vector_total_c(vec)
#define VECTOR_FREE_C(vec) vector_free_c(&vec)
#define VECTOR_FREE_C_EX(vec) vector_free_c(vec)

typedef struct _vector_c {
    void **items;
    int capacity;
    int total;
} vector_c;

void vector_init_c(vector_c *);
int vector_total_c(vector_c *);
static void vector_resize_c(vector_c *, int);
void vector_add_c(vector_c *, void *);
void vector_set_c(vector_c *, int, void *);
void *vector_get_c(vector_c *, int);
void vector_delete_c(vector_c *, int);
void vector_free_c(vector_c *);

#endif