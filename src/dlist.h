#include <assert.h>
#include <stddef.h>
#include <stdbool.h>

#ifndef __DLIST_H_
#define __DLIST_H_

#define DLIST_INIT { .prev = NULL, .next = NULL }

typedef struct _DList DList;

typedef bool (*DListIter)(DList *node, void *data);
typedef bool (*DListComp)(DList *n1, void *data);
typedef void (*DListFree)(DList *node);

struct _DList {
  DList *prev;
  DList *next;
};

inline bool dlist_empty(DList *self) { return self == NULL; }

#define dlist_prev(type, self) ((type *) _dlist_prev((DList *) (self)))

static inline DList *_dlist_prev(DList *self) {
  return self ? self->prev : NULL;
}

#define dlist_next(type, self) ((type *) _dlist_next((DList *) (self)))

static inline DList *_dlist_next(DList *self) {
  return self ? self->next : NULL;
}

#define dlist_remove(n) _dlist_remove((DList *) (n))

void _dlist_remove(DList *self);

/**
 * 將 `self` 加到 `node` 之前
 * `self` 可以是 `NULL`
 * `*node` 可以是 `NULL`
 */
#define dlist_prepend(self, head) _dlist_prepend((DList *) (self), (DList **)(&head))

void _dlist_prepend(DList *self, DList **node);

/**
 * 將 `self` 加到 `node` 之後
 */
#define dlist_append(self, head) _dlist_append((DList *) (self), (DList **)(&head))

void _dlist_append(DList *self, DList **node);

/**
 * free single node
 */
#define dlist_free(self, func) _dlist_free((DList *) (self), (DListFree)(func))

static inline void _dlist_free(DList *self, DListFree func) {
  func(self);
}

#define dlist_foreach(head, func, data) _dlist_foreach((DList *) (head), \
                                                       (DListIter)(func), \
                                                       (void *) (data))

static inline void _dlist_foreach(DList *head, DListIter func, void *data) {
  if(!head) { return; }

  DList *node = head;
  do {
    DList *next = node->next;
    if(!func(node, data)) {
      break;
    }
    node = next;
  } while(node && node != head);
}

#define dlist_free_all(head, func) _dlist_free_all((DList *) (head), (DListFree)(func))

static bool free_node(DList *node, DListFree func) {
  func(node);
  return true;
}

static inline void _dlist_free_all(DList *head, DListFree func) {
  dlist_foreach(head, free_node, func);
}

#endif /* __DLIST_H_ */
