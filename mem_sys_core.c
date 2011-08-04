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
extern VALUE rb_newobj_core();
extern void rb_gc_core();
extern void rb_gc_mark_core(VALUE object);
extern void rb_gc_mark_locations_core(VALUE *start, VALUE *end);
extern int  rb_gc_markedQ_core(VALUE object);
extern void rb_gc_register_address_core(VALUE *addr);
extern void rb_gc_unregister_address_core(VALUE *addr);

extern void rb_gc_at_exit_core();
extern void Init_GC_core();

rb_mem_sys rb_mem_sys_core = {
  "core",
  0,
  0, /* initialize */
  0, /* options */
  0, /* Init_GC */
  rb_newobj_core,
  rb_gc_core,
  rb_gc_mark_core,
  rb_gc_mark_locations_core,
  rb_gc_markedQ_core,
  rb_gc_register_address_core,
  rb_gc_unregister_address_core,
  rb_gc_at_exit_core,
};

