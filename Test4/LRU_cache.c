#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define container_of(ptr, type, member) \
    ((type *) ((char *) (ptr) - (size_t) & (((type *) 0)->member)))

#define list_entry(ptr, type, member) container_of(ptr, type, member)

#define hlist_for_each(pos, head) \
    for (pos = (head)->first; pos; pos = pos->next)

#define hlist_for_each_safe(pos, n, head)        \
    for (pos = (head)->first; pos && ({          \
                                  n = pos->next; \
                                  true           \
                              });                \
         pos = n)

#define list_first_entry(ptr, type, field) list_entry((ptr)->next, type, field)
#define list_last_entry(ptr, type, field) list_entry((ptr)->prev, type, field)

#define list_for_each(p, head) for (p = (head)->next; p != head; p = p->next)

#define list_for_each_safe(p, n, head) \
    for (p = (head)->next, n = p->next; p != (head); p = n, n = p->next)

struct hlist_node;

struct hlist_head {
    struct hlist_node *first;
};

struct hlist_node {
    struct hlist_node *next, **pprev;
};

void INIT_HLIST_HEAD(struct hlist_head *h)
{
    h->first = NULL;
}

void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
    if (h->first)
        h->first->pprev = &n->next;
    n->next = h->first;
    n->pprev = &h->first;
    h->first = n;
}

void hlist_del(struct hlist_node *n)
{
    struct hlist_node *next = n->next, **pprev = n->pprev;
    *pprev = next;
    if (next)
        next->pprev = pprev;
}

struct list_head {
    struct list_head *next, *prev;
};

void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list->prev = list;
}

void __list_add(struct list_head *new,
                struct list_head *prev,
                struct list_head *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

void list_add(struct list_head *_new, struct list_head *head)
{
    __list_add(_new, head, head->next);
}

void __list_del(struct list_head *entry)
{
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
}

void list_del(struct list_head *entry)
{
    __list_del(entry);
    entry->next = entry->prev = NULL;
}

void list_move(struct list_head *entry, struct list_head *head)
{
    __list_del(entry);
    list_add(entry, head);
}

typedef struct {
    int capacity;
    int count;
    struct list_head dhead;
    struct hlist_head hhead[];
} LRUCache;

typedef struct {
    int key;
    int value;
    struct hlist_node node;
    struct list_head link;
} LRUNode;

LRUCache *lRUCacheCreate(int capacity)
{
    LRUCache *cache = malloc(2 * sizeof(int) + sizeof(struct list_head) +
                             capacity * sizeof(struct list_head));
    cache->capacity = capacity;
    cache->count = 0;
    INIT_LIST_HEAD(&cache->dhead);
    for (int i = 0; i < capacity; i++)
        INIT_HLIST_HEAD(&cache->hhead[i]);
    return cache;
}

void lRUCacheFree(LRUCache *obj)
{
    struct list_head *pos, *n;
    list_for_each_safe (pos, n, &obj->dhead) {
        LRUNode *cache = list_entry(pos, LRUNode, link);
        list_del(&cache->link);
        free(cache);
    }
    free(obj);
}

int lRUCacheGet(LRUCache *obj, int key)
{
    int hash = key % obj->capacity;
    struct hlist_node *pos;
    hlist_for_each (pos, &obj->hhead[hash]) {
        LRUNode *cache = list_entry(pos, LRUNode, node);
        if (cache->key == key) {
            list_move(&cache->link, &obj->dhead);
            return cache->value;
        }
    }
    return -1;
}

void lRUCachePut(LRUCache *obj, int key, int value)
{
    LRUNode *cache = NULL;
    int hash = key % obj->capacity;
    struct hlist_node *pos;
    hlist_for_each (pos, &obj->hhead[hash]) {
        LRUNode *c = list_entry(pos, LRUNode, node);
        if (c->key == key) {
            list_move(&c->link, &obj->dhead);
            cache = c;
        }
    }

    if (!cache) {
        if (obj->count == obj->capacity) {
            cache = list_last_entry(&obj->dhead, LRUNode, link);
            list_move(&cache->link, &obj->dhead);
            hlist_del(&cache->node);
            hlist_add_head(&cache->node, &obj->hhead[hash]);
        } else {
            cache = malloc(sizeof(LRUNode));
            hlist_add_head(&cache->node, &obj->hhead[hash]);
            list_add(&cache->link, &obj->dhead);
            obj->count++;
        }
        cache->key = key;
    }
    cache->value = value;
}

int main(void)
{
    int capacity = 2;
    LRUCache *cache = lRUCacheCreate(capacity);
    
    printf("Input: {key:value} = {1, 1}\n");
    lRUCachePut(cache, 1, 1);
    printf("Input: {key:value} = {2, 2}\n");
    lRUCachePut(cache, 2, 2);
    printf("Output: {key:value} = {1, %d}\n", lRUCacheGet(cache, 1));
    printf("Input: {key:value} = {3, 3}\n");
    lRUCachePut(cache, 3, 3);
    printf("Output: {key:value} = {2, %d}\n", lRUCacheGet(cache, 2));
    printf("Input: {key:value} = {4, 4}\n");
    lRUCachePut(cache, 4, 4);
    printf("Output: {key:value} = {1, %d}\n", lRUCacheGet(cache, 1));
    printf("Output: {key:value} = {3, %d}\n", lRUCacheGet(cache, 3));
    printf("Output: {key:value} = {4, %d}\n", lRUCacheGet(cache, 4));

    return 0;
}
