/**********************************************************************

  mem_api.h - Memory/GC internals API.

  Author: Kurt Stephens

  Copyright (C) 2010, 2011 Kurt Stephens

**********************************************************************/
#ifndef RUBY_MEM_API_H
#define RUBY_MEM_API_H

#if defined(__cplusplus)
extern "C" {
#if 0
} /* satisfy cc-mode */
#endif
#endif

#include "ruby/defines.h"
#ifdef RUBY_EXTCONF_H
#include RUBY_EXTCONF_H
#endif

#if defined __GNUC__ && __GNUC__ >= 4
#pragma GCC visibility push(default)
#endif

/* Memory system object. */
typedef struct rb_mem_sys {
  const char *name;
  void *data;
  void (*initialize)(struct rb_mem_sys *ms);
  void (*options)(struct rb_mem_sys *ms, const char *options);
  void (*Init_GC)();
  /* API methods. */
  VALUE (*newobj)(void);
  void (*gc)(void);
  void (*gc_mark)(VALUE object);
  void (*gc_mark_locations)(VALUE *start, VALUE *end);
  int  (*gc_markedQ)(VALUE object);
  void (*gc_register_address)(VALUE *addr);
  void (*gc_unregister_address)(VALUE *addr);
  void (*gc_define_finalizer)(VALUE obj, VALUE block);
  void (*gc_undefine_finalizer)(VALUE obj);
  void (*gc_at_exit)();
  struct rb_mem_sys *next; /* mem_sys_list */
  const char *opts;
} rb_mem_sys;

extern size_t rb_sizeof_RVALUE; /* HACK!!! */

void rb_mem_sys_init(); /* Initialize memory system API. */
void rb_mem_sys_select(const char *name); /* Select memory system by name, or 0 to use $RUBY_MEM_SYS, else rb_mem_sys_default. */
void rb_mem_sys_register(rb_mem_sys *); /* Register a memory system. */
extern const char *rb_mem_sys_default;

/* Internal mem system APIs (as defined in gc.c) */
#if 0
VALUE rb_newobj(void);
void rb_gc(void);
void rb_gc_mark(VALUE);
void rb_gc_mark_locations(VALUE *start, VALUE *end);
#endif
void rb_gc_define_finalizer(VALUE obj, VALUE block);
void rb_gc_undefine_finalizer(VALUE obj);
void rb_gc_run_finalizer(VALUE obj, VALUE block);

/* Additional mem system APIs for GC callbacks. */
extern int rb_gc_markedQ(VALUE object);

/*
 * Memory System callback API.
 */

#ifndef RB_MEM_SYS_TRACE_EVENTS
#define RB_MEM_SYS_TRACE_EVENTS 1
#endif

/* Event support. */
enum rb_mem_sys_event {
  RB_MEM_SYS_EVENT_NONE = 0,
  RB_MEM_SYS_EVENT_STRESS,
  RB_MEM_SYS_EVENT_OBJECT_ALLOC,
  RB_MEM_SYS_EVENT_OBJECT_FREE,
  RB_MEM_SYS_EVENT_PAGE_ALLOC,
  RB_MEM_SYS_EVENT_PAGE_FREE,
  RB_MEM_SYS_EVENT_FINALIZER_ALLOC,
  RB_MEM_SYS_EVENT_FINALIZER_FREE,
  RB_MEM_SYS_EVENT_GC_START,
  RB_MEM_SYS_EVENT_GC_MARK,
  RB_MEM_SYS_EVENT_GC_SWEEP,
  RB_MEM_SYS_EVENT_GC_FINALIZE,
  RB_MEM_SYS_EVENT_GC_END,
  RB_MEM_SYS_EVENT_AT_EXIT,
  rb_mem_sys_event__LAST
};
enum rb_mem_sys_event_location {
  RB_MEM_SYS_EVENT_BEFORE = 0,
  RB_MEM_SYS_EVENT_AFTER,
  rb_mem_sys_event_location__LAST
};

/* Returns an opaque callback struct. */
void *rb_mem_sys_add_callback(enum rb_mem_sys_event phase, 
			      enum rb_mem_sys_event_location location, 
			      void (*func)(void *callback, void *func_data, void *data, size_t size), 
			      void *func_data);
void rb_mem_sys_set_callback_func(void *callback, void *func);
void rb_mem_sys_remove_callback(void *callback);
void rb_mem_sys_invoke_callbacks(enum rb_mem_sys_event phase, 
				 enum rb_mem_sys_event_location location, 
				 void *addr, 
				 size_t size);

#if defined __GNUC__ && __GNUC__ >= 4
#pragma GCC visibility pop
#endif

#if defined(__cplusplus)
#if 0
{ /* satisfy cc-mode */
#endif
}  /* extern "C" { */
#endif

#endif
