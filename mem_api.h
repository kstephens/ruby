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

/* Defines memory system object. */
typedef struct rb_mem_sys {
  const char *name;
  void *data;
  void (*initialize)(struct rb_mem_sys *ms);
  void (*options)(struct rb_mem_sys *ms, const char *options);
  /* API methods. */
  VALUE (*newobj)(void);
  void (*gc)(void);
  void (*gc_mark)(VALUE object);
  void (*gc_mark_locations)(VALUE *start, VALUE *end);
  int  (*gc_markedQ)(VALUE object);
  void  (*gc_at_exit)();
  struct rb_mem_sys *next; /* mem_sys_list */
} rb_mem_sys;

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
/* Addtional mem system APIs for GC callbacks. */
extern int rb_gc_markedQ(VALUE object);

enum rb_gc_phase {
  RB_GC_PHASE_NONE = 0,
  RB_GC_PHASE_STRESS,
  RB_GC_PHASE_ALLOC,
  RB_GC_PHASE_START,
  RB_GC_PHASE_MARK,
  RB_GC_PHASE_SWEEP,
  RB_GC_PHASE_FINALIZE,
  RB_GC_PHASE_END,
  RB_GC_PHASE_AT_EXIT,
  rb_gc_phase__LAST
};
enum rb_gc_phase_location {
  RB_GC_PHASE_BEFORE = 0,
  RB_GC_PHASE_AFTER,
  rb_gc_phase_location__LAST
};

/*
 * GC callback API.
 */
/* Returns an opaque callback struct. */
void *rb_gc_add_callback(enum rb_gc_phase phase, enum rb_gc_phase_location location, void (*func)(void *callback, void *func_data), void *func_data);
void rb_gc_set_callback_func(void *callback, void *func);
void rb_gc_remove_callback(void *callback);
void rb_gc_invoke_callbacks(enum rb_gc_phase phase, enum rb_gc_phase_location location);

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
