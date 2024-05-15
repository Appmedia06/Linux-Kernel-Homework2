#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>



typedef struct __node {
    // struct __node *left, *right;
    struct __node *next;
    int value;
} node_t;

void list_add(node_t **list, node_t *node_t)
{
    node_t->next = *list;
    *list = node_t;
}

node_t *list_tail(node_t **left)
{
    while ((*left) && (*left)->next)
        left = &((*left)->next);
    return *left;
}

int list_length(node_t **left)
{
    int n = 0;
    while (*left) {
        ++n;
        left = &((*left)->next);
    }
    return n;
}

node_t *list_construct(node_t *list, int n)
{
    node_t *node = malloc(sizeof(node_t));
    node->next = list;
    node->value = n;
    return node;
}

void list_free(node_t **list)
{
    node_t *node = (*list)->next;
    while (*list) {
        free(*list);
        *list = node;
        if (node)
            node = node->next;
    }
}

/* Verify if list is order */
static bool list_is_ordered(node_t *list)
{       
    bool first = true;
    int value;
    while (list) {
        if (first) {
            value = list->value;
            first = false;
        } else {
            if (list->value < value)
                return false;
            value = list->value;
        }
        list = list->next;
    }
    return true;
}

/* shuffle array, only work if n < RAND_MAX */
void shuffle(int *array, size_t n)
{
    if (n <= 0)
        return;

    for (size_t i = 0; i < n - 1; i++) {
        size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
        int t = array[j];
        array[j] = array[i];
        array[i] = t;
    }
}

void swap_node(node_t *a, node_t *b)
{
    node_t *tmp;
    tmp = a->next;
    a->next = b->next;
    b->next = tmp;
}

void select_sort(int *array)
{
    int i, j, key;
    for (i = 1; i < 3; i++) {
        key = array[i];
        j = i - 1;
        while (j >= 0 && array[j] > key) {
            array[j + 1] = array[j];
            j--;
        }
        array[j + 1] = key;
    }
}

void select_sort_list(node_t *list)
{
    node_t *i, *j;
    int tmp;
    for (i = list; i != NULL; i = i->next) {
        for (j = i->next; j != NULL; j = j->next) {
            if (i->value > j->value) {
                tmp = i->value;
                i->value = j->value;
                j->value = tmp;
            }
        }
    }
}

void rand_pivot(node_t *list)
{
   if (!list)
       return;

   int rand_index = rand() % list_length(&list);

   node_t *iter = list;
   while (iter && iter->next) {
       if (rand_index == 0) {
            break;
       }
       iter = iter->next;
       rand_index--;
   }
   swap_node(list, iter);
}

void delete_remain(int *arr, int n)
{
    int value = arr[0], tmp;
    for (int i = 1; i < n; i++) {
        tmp = arr[i];
        arr[i] -= value;
        value += tmp;
    }
}

void rand_threeMedian_pivot(node_t *list)
{
    if (!list)
        return;

    int rand_index[3] = {0};
    int len = list_length(&list), i;
    for (i = 0; i < 3; i++) {
        rand_index[i] = rand() % len;
    }
    select_sort(rand_index);
    delete_remain(rand_index, 3);

    node_t *iter = list, *tmp = NULL;
    i = 0;
    while (iter && iter->next) {
        if (rand_index[i] <= 0) {
           node_t *new_node = (node_t*) malloc(sizeof(*new_node));
           new_node->value = iter->value;
           list_add(&tmp, new_node);
           i += 1;
           if (i >= 3) {
               break;
           }
        }
        iter = iter->next;
        rand_index[i]--;
    }
    
    select_sort_list(tmp);
    node_t *rand_node = tmp;
    swap_node(list, rand_node);
}

void quick_sort(node_t **list)
{
    int n = list_length(list);
    int value;
    int i = 0;
    int max_level = 2 * n;
    node_t *begin[max_level], *end[max_level];
    node_t *result = NULL, *left = NULL, *right = NULL;

    begin[0] = *list;
    end[0] = list_tail(list);

    while (i >= 0) {
        node_t *L = begin[i], *R = end[i];
        if (L != R) {
            rand_threeMedian_pivot(L);
            node_t *pivot = L;
            value = pivot->value;
            node_t *p = pivot->next;
            pivot->next = NULL;

            while (p) {
                node_t *n = p;
                p = p->next;
                list_add(n->value > value ? &right : &left, n);
            }

            begin[i] = left;
            end[i] = list_tail(&left);
            begin[i + 1] = pivot;
            end[i + 1] = pivot;
            begin[i + 2] = right;
            end[i + 2] = list_tail(&right);

            left = right = NULL;
            i += 2;
        } else {
            if (L)
                list_add(&result, L);
            i--;
        }
    }
    *list = result;
}

int main(int argc, char **argv)
{
    node_t *list = NULL;

    size_t count = 10000;

    int *test_arr = malloc(sizeof(int) * count);

    for (int i = 0; i < count; ++i)
        test_arr[i] = i;
    shuffle(test_arr, count);

    while (count--)
        list = list_construct(list, test_arr[count]);

    select_sort_list(list);
    assert(list_is_ordered(list));

    quick_sort(&list);
    assert(list_is_ordered(list));

    list_free(&list);

    free(test_arr);

    return 0;
}
