/**********************************************************************

  mem_sys_core.h - Standard Ruby GC memory system.

  Author: Kurt Stephens
  created at: 2011/08/03

  Copyright (C) 2011 Kurt Stephens

**********************************************************************/
#include "ruby.h"
#include "mem_api.h"
#include <stdlib.h>

/* Defined in gc.c */
extern void *ruby_xmalloc_core(size_t n);
extern void ruby_xfree_core(void *x);
extern void *ruby_xrealloc_core(void *ptr, size_t size);
extern void *ruby_xcalloc_core(size_t, size_t);
extern VALUE rb_newobj_core();
extern void rb_gc_core();
extern void rb_gc_mark_core(VALUE object);
extern void rb_gc_mark_locations_core(VALUE *start, VALUE *end);
extern int  rb_gc_markedQ_core(VALUE object);
extern void rb_gc_register_address_core(VALUE *addr);
extern void rb_gc_unregister_address_core(VALUE *addr);
extern void rb_gc_define_finalizer_core(VALUE obj, VALUE proc);
extern void rb_gc_undefine_finalizer_core(VALUE obj);

extern void rb_gc_at_exit_core();
extern void Init_GC_core();
extern void Init_heap_core();

rb_mem_sys rb_mem_sys_core = {
  "core",
  0,
  0, /* initialize */
  0, /* options */
  0, /* Init_GC */
  Init_heap_core,
  ruby_xmalloc_core,
  ruby_xfree_core,
  ruby_xrealloc_core,
  ruby_xcalloc_core,
  rb_newobj_core,
  rb_gc_core,
  rb_gc_mark_core,
  rb_gc_mark_locations_core,
  rb_gc_markedQ_core,
  rb_gc_register_address_core,
  rb_gc_unregister_address_core,
  rb_gc_define_finalizer_core,
  rb_gc_undefine_finalizer_core,
  rb_gc_at_exit_core,
};

