/**********************************************************************

  mem_api.c - Memory/GC internals API.

  Author: Kurt Stephens
  created at: Mon Jan 17 12:09:32 CST 2011

  Copyright (C) 2011 Kurt Stephens, Enova Financial
*/

#include "ruby.h"
#include "mem_api.h"
#include <stdlib.h>

extern void rb_newobj_core();
extern void rb_gc_core();
extern void rb_gc_mark_core(VALUE object);
extern void rb_gc_mark_locations_core(VALUE *start, VALUE *end);
extern int  rb_gc_markedQ_core(VALUE object);
static rb_mem_sys ms = {
  "core",
  0,
  rb_newobj_core,
  rb_gc_core,
  rb_gc_mark_core,
  rb_gc_mark_locations_core,
  rb_gc_markedQ_core,
};

VALUE rb_newobj(void)
{
  return ms.newobj();
}

void rb_gc(void)
{
  ms.gc();
}

void rb_gc_mark(VALUE obj)
{
  ms.gc_mark(obj);
}

void rb_gc_mark_locations(VALUE *start, VALUE *end)
{
  ms.gc_mark_locations(start, end);
}

int rb_gc_markedQ(VALUE obj)
{
  return ms.gc_markedQ(obj);
}

typedef struct rb_gc_callback {
  struct rb_gc_callback *next, *prev;
  enum rb_gc_phase phase;
  enum rb_gc_phase_location location;
  void (*func)(void *, void*);
  void *func_data;
} rb_gc_callback;

static rb_gc_callback callbacks[rb_gc_phase__LAST][rb_gc_phase_location__LAST];

void *rb_gc_add_callback(enum rb_gc_phase phase, enum rb_gc_phase_location location, void (*func)(void *callback, void *func_data), void *func_data)
{
  rb_gc_callback *cb = xmalloc(sizeof(*cb));
  cb->phase = phase;
  cb->location = location;
  cb->func = func;
  cb->func_data = func_data;
  {
    rb_gc_callback *pos = &callbacks[phase][location];
    /* Initialize linked list head. */
    if ( ! pos->next )
      pos->next = pos->prev = pos;
    pos = pos->prev;
    cb->prev = pos;
    cb->next = pos->next;
    pos->next->prev = cb;
    pos->next = cb;
  }
  return cb;
}

void rb_gc_remove_callback(void *callback)
{
  rb_gc_callback *cb = callback;
  cb->func = 0; /* guard. */
  cb->next->prev = cb->prev;
  cb->prev->next = cb->next;
  xfree(cb);
}

void rb_gc_set_callback_func(void *callback, void *func)
{
  rb_gc_callback *cb = callback;
  cb->func = func;
}

void rb_gc_invoke_callbacks(enum rb_gc_phase phase, enum rb_gc_phase_location location)
{
  rb_gc_callback *end = &callbacks[phase][location]; 
  rb_gc_callback *cb = end->next;
  int zero_words[128];
  /* Uninitialized and empty linked list head. */
  if ( cb && cb != end ) {
    int func_called = 0;
    do {
      /* Callback func may invoke rb_gc_remove_callback() on itself. */
      rb_gc_callback *cb_next = cb->next; 
      if ( cb->func ) {
	func_called = 1;
	cb->func(cb, cb->func_data);
      }
      cb = cb_next;
    } while ( cb != end );
    /* Avoid garbage on stack. */
    if ( func_called ) 
      memset(zero_words, 0, sizeof(zero_words));
  }
}

