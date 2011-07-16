/**********************************************************************

  gc_api.h - GC internals API.

  $Author$
  created at: Mon Jan 17 12:09:32 CST 2011

  Copyright (C) 2010 Kurt Stephens

**********************************************************************/
#ifndef RUBY_GC_API_H
#define RUBY_GC_API_H

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

void rb_gc_mark(VALUE);

#ifdef MBARI_API
/* #warning "Using REE/MBARI rb_mark_table_contains" */
extern int rb_gc_markedQ(VALUE object);
#define RB_GC_MARKED(X) (FL_ABLE((X)) ? rb_gc_markedQ((X)) : 0)
#endif

#ifndef RB_GC_MARKED
#define RB_GC_MARKED(X) FL_TEST((X), FL_MARK)
#endif

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
