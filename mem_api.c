/**********************************************************************

  mem_api.c - Memory/GC internals API.

  Author: Kurt Stephens

  Copyright (C) 2010, 2011 Kurt Stephens
*/

#include "ruby.h"
#include "mem_api.h"
#include <string.h>
#include <stdlib.h>

static rb_mem_sys ms; /* The active, selected rb_mem_sys. */
static rb_mem_sys *mem_sys_list = 0; /* List of registered rb_mem_sys objects. */

void rb_mem_sys_register(rb_mem_sys *mem_sys)
{
  mem_sys->next = mem_sys_list;
  mem_sys_list = mem_sys;
}

void rb_mem_sys_init()
{
#define MEM_SYS(N)				\
  {						\
    extern rb_mem_sys rb_mem_sys_##N;		\
    rb_mem_sys_register(&rb_mem_sys_##N);	\
  }
  MEM_SYS(core);
  MEM_SYS(malloc);
#undef MEM_SYS
}

const char *rb_mem_sys_default = "core";

void rb_mem_sys_select(const char *name)
{
  if ( ! (name && *name) )
    name = getenv("RUBY_MEM_SYS");

  if ( ! (name && *name) ) 
    name = rb_mem_sys_default;

  {
    const char *options = strchr(name, ':');
    size_t name_len;
    rb_mem_sys *p = mem_sys_list;
    if ( ! options )
      options = strchr(name, '\0');
    name_len = options - name;
    while ( p ) {
      if ( ! strncmp(p->name, name, name_len) )
	break;
      p = p->next;
    }
    if ( p ) {
      ms = *p;
      if ( ms.initialize )
	ms.initialize(&ms);
      if ( ms.options && *options )
	ms.options(&ms, options);
      // fprintf(stderr, "\nrb_mem_sys_select: selected %s\n", ms.name);
    } else {
      rb_fatal("rb_mem_sys_select: cannot locate %s", name);
      abort();
    }
  }
}

void Init_mem_sys()
{
  rb_mem_sys_init();
  rb_mem_sys_select(0);
}

/********************************************************************
 * Internal interface to rb_mem_sys methods.
 */

VALUE rb_newobj(void)
{
  VALUE obj = ms.newobj();
#ifdef GC_DEBUG
  RANY(obj)->file = rb_sourcefile();
  RANY(obj)->line = rb_sourceline();
#endif
  return obj;
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

void
rb_gc_register_address(VALUE *addr)
{
  ms.gc_register_address(addr);
}

void
rb_gc_unregister_address(VALUE *addr)
{
  ms.gc_unregister_address(addr);
}

void 
rb_gc_define_finalizer(VALUE obj, VALUE proc)
{
  ms.gc_define_finalizer(obj, proc);
}

void 
rb_gc_undefine_finalizer(VALUE obj)
{
  ms.gc_undefine_finalizer(obj);
}

void rb_gc_at_exit()
{
  if ( ms.gc_at_exit )
    ms.gc_at_exit();
}

/* See eval.c */
void rb_gc_call_finalizer_at_exit(void)
{
  rb_gc_at_exit();
}

/********************************************************************/

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

