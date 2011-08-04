/**********************************************************************

  mem_sys_malloc.h - Malloc-only GC memory system.

  Author: Kurt Stephens
  created at: 2011/08/03

  Copyright (C) 2011 Kurt Stephens

**********************************************************************/
#include "ruby.h"
#include "mem_api.h"
#include <stdlib.h>

extern size_t rb_sizeof_RVALUE; /* UGLY */
static VALUE rb_newobj_malloc()
{
  void *ptr;
  ptr = malloc(rb_sizeof_RVALUE);
  bzero(ptr, rb_sizeof_RVALUE);
  return (VALUE) ptr;
}

static int show_line;

/***************************
 * Use RUBY_MEM_SYS=malloc:D to get allocation tracing.
 * Use RUBY_MEM_SYS=malloc:DL to get allocation tracing with file:line.
 */
static VALUE rb_newobj_malloc_debug()
{
  static size_t alloc_id = 0;
  static size_t total_size = 0;
  VALUE obj = rb_newobj_malloc();
  ++ alloc_id;
  total_size += rb_sizeof_RVALUE;
  fprintf(stderr, "  rb_newobj_malloc: pid=%d alloc_id=%lu total_size=%lu ptr=%p", (int) getpid(), (unsigned long) alloc_id, (unsigned long) total_size, (void*) obj);
  if ( show_line )
    fprintf(stderr, " @%s:%d", rb_sourcefile(), rb_sourceline());
  fprintf(stderr, "\n");
  return obj;
}

static void rb_gc_malloc()
{
  /* NOTHING */
}

static void rb_gc_mark_malloc(VALUE object)
{
  /* NOTHING */
}

static void rb_gc_mark_locations_malloc(VALUE *start, VALUE *end)
{
  /* NOTHING */
}

static int  rb_gc_markedQ_malloc(VALUE object)
{
  return 0;
}

static void mem_sys_malloc_options(rb_mem_sys *ms, const char *options)
{
  if ( strchr(options, 'D') ) {
    ms->newobj = rb_newobj_malloc_debug;
    show_line = ! ! strchr(options, 'L');
  }
}

rb_mem_sys rb_mem_sys_malloc = {
  "malloc",
  0,
  0,
  mem_sys_malloc_options,
  rb_newobj_malloc,
  rb_gc_malloc,
  rb_gc_mark_malloc,
  rb_gc_mark_locations_malloc,
  rb_gc_markedQ_malloc,
};

