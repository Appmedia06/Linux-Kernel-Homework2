#ifndef SORT_IMPL_H
#define SORT_IMPL_H

#include "list.h"
// #include "timsort.c"

struct list_head;

typedef int (*list_cmp_func_t)(void *,
                               const struct list_head *,
                               const struct list_head *);

void timsort(void *priv, struct list_head *head, list_cmp_func_t cmp);

#endif
