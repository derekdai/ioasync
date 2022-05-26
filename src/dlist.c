#include "dlist.h"

void _dlist_remove(DList *self) {
  if(!self) { return; }

  if(self->prev == self) {
    self->prev = NULL;
  } else if(self->prev) {
    self->prev->next = self->next;
  }

  if(self->next == self) {
    self->next = NULL;
  } else if(self->next) {
    self->next->prev = self->prev;
  }
}

static inline void dlink_init(DList *self) {
  self->prev = self;
  self->next = self;
}

void _dlist_prepend(DList *self, DList **node) {
  assert(node);

  if(!self) { return; }

  if(!*node) {
    *node = self;
    dlink_init(*node);
  } else if(*node == (*node)->prev) {
    (*node)->next = self;
    (*node)->prev = self;
    self->prev = *node;
    self->next = *node;
  } else {
    (*node)->prev->next = self;
    self->prev = (*node)->prev;

    (*node)->prev = self;
    self->next = (*node);
  }
}

/**
 * 將 `self` 加到 `node` 之後
 */
#define dlist_append(self, head) _dlist_append((DList *) (self), (DList **)(&head))

void _dlist_append(DList *self, DList **node) {
  assert(node);

  if(!self) { return; }

  if(!*node) {
    *node = self;
    dlink_init(*node);
  } else if(*node == (*node)->next) {
    (*node)->next = self;
    (*node)->prev = self;
    self->prev = *node;
    self->next = *node;
  } else {
    (*node)->next->prev = self;
    self->next = (*node)->next;

    (*node)->next = self;
    self->prev = (*node);
  }
}
