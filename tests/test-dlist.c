#include "../src/dlist.h"
#include "../src/utils.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#define assert_neq(v1, v2) assert((v1) != (v2))
#define assert_eq(v1, v2) assert((v1) == (v2))

typedef struct Value Value;

struct Value {
  DList list;
  int v;
};

Value *value_new(int v) {
  Value *self = malloc(sizeof(Value));
  *self = (Value) { .list = DLIST_INIT, .v = v };
  return self;
}

#define dump(v) _dump((v), #v)

void _dump(Value *v, const char *name) {
  printf("%s(%p) {.v=%d, .prev=%p, .next=%p}\n", name, v, v->v, v->list.prev, v->list.next);
}

int main() {
  {
    Value *v = value_new(123);
    assert_neq(v, NULL);
    assert_eq(v->v, 123);
    assert_eq(dlist_next(Value, v), NULL);
    assert_eq(dlist_prev(Value, v), NULL);
    dlist_free(v, free);
  }

  {
    Value *head = NULL;
    dlist_append(value_new(123), head);
    assert_neq(head, NULL);
    assert_eq(head->v, 123);
    assert_eq(dlist_prev(Value, head), head);
    assert_eq(dlist_next(Value, head), head);

    Value *v2 = value_new(456);
    dlist_append(v2, head);
    assert_neq(head, v2);
    assert_eq(dlist_next(Value, head), v2);
    assert_eq(dlist_prev(Value, head), v2);
    assert_eq(dlist_next(Value, head)->v, 456);
    assert_eq(dlist_prev(Value, head)->v, 456);

    Value *v3 = value_new(789);
    dlist_append(v3, head);
    assert_neq(head, v2);
    assert_neq(head, v3);
    assert_eq(dlist_next(Value, head), v3);
    assert_eq(dlist_prev(Value, head), v2);
    assert_eq(dlist_next(Value, head)->v, 789);
    assert_eq(dlist_prev(Value, head)->v, 456);

    dlist_remove(v2);
    assert_neq(head, v2);
    assert_neq(head, v3);
    assert_eq(dlist_next(Value, head), v3);
    assert_eq(dlist_prev(Value, head), v3);
    assert_eq(dlist_next(Value, v3), head);
    assert_eq(dlist_prev(Value, v3), head);
    assert_eq(head->v, 123);
    assert_eq(dlist_next(Value, head)->v, 789);
    assert_eq(dlist_prev(Value, head)->v, 789);

    dlist_prepend(v2, head);
    assert_neq(head, v2);
    assert_neq(head, v3);
    assert_eq(dlist_next(Value, head), v3);
    assert_eq(dlist_prev(Value, head), v2);
    assert_eq(dlist_next(Value, v3), v2);
    assert_eq(dlist_prev(Value, v3), head);
    assert_eq(dlist_next(Value, v2), head);
    assert_eq(dlist_prev(Value, v2), v3);

    //dlist_free(v3, free);
    //dlist_free(v2, free);
    //dlist_free(head, free);
    dlist_free_all(head, free);
  }
}
