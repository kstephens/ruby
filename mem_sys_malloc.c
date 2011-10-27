/**********************************************************************

  mem_sys_malloc.h - Malloc-only memory system.

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
  if ( ! ptr ) {
    rb_memerror();
    return (VALUE) ptr; /* NOTREACHED? */
  }
  bzero(ptr, rb_sizeof_RVALUE);
  return (VALUE) ptr;
}

static int show_trace;
static int show_line;
static int show_total_at_exit;
static int show_register_address;

/***************************
 * RUBY_MEM_SYS=malloc:D  enables allocation tracing.
 * RUBY_MEM_SYS=malloc:DL enables allocation tracing with @file:line.
 * RUBY_MEM_SYS=malloc:E  enables total alloc counts and size at exit.
 * RUBY_MEM_SYS=malloc:R  enables rb_gc_(un)register_address() tracing.
 */

static size_t alloc_id = 0;
static size_t total_size = 0;
static VALUE rb_newobj_malloc_debug()
{
  VALUE obj = rb_newobj_malloc();
  ++ alloc_id;
  total_size += rb_sizeof_RVALUE;
  if ( show_trace ) {
    fprintf(stderr, "  rb_newobj_malloc: pid=%d alloc_id=%lu total_size=%lu ptr=%p", (int) getpid(), (unsigned long) alloc_id, (unsigned long) total_size, (void*) obj);
    if ( show_line )
      fprintf(stderr, " @%s:%d", rb_sourcefile(), rb_sourceline());
    fprintf(stderr, "\n");
  }
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

static void
rb_gc_register_address_malloc(VALUE *addr)
{
  if ( show_register_address ) {
    fprintf(stderr, "\n  rb_gc_register_address_malloc: pid=%d alloc_id=%lu addr=%p", (int) getpid(), (unsigned long) alloc_id, (void*) addr);
    //if ( show_line )
    //  fprintf(stderr, " @%s:%d\n", rb_sourcefile(), rb_sourceline());
    fprintf(stderr, "\n");
  }
}

static void
rb_gc_unregister_address_malloc(VALUE *addr)
{
  if ( show_register_address ) {
    fprintf(stderr, "\n  rb_gc_unregister_address_malloc: pid=%d alloc_id=%lu addr=%p", (int) getpid(), (unsigned long) alloc_id, (void*) addr);
    //if ( show_line )
    //  fprintf(stderr, " @%s:%d\n", rb_sourcefile(), rb_sourceline());
    fprintf(stderr, "\n");
  }
}

static struct finalizer {
  VALUE obj, block;
  struct finalizer *next;
} *finalizers;

static
void rb_gc_define_finalizer_malloc(VALUE obj, VALUE block)
{
  struct finalizer *f = malloc(sizeof(*f));
  fprintf(stderr, "\n  rb_gc_define_finalizer_malloc(%p, %p)\n", (void*) obj, (void*) block);
  f->obj = obj;
  f->block = block;
  f->next = finalizers;
  finalizers = f;
}

static
void rb_gc_undefine_finalizer_malloc(VALUE obj)
{
  struct finalizer **fp = &finalizers, *f;
  while ( (f = *fp) ) {
    void *f_next = &(f->next);
    if ( f->obj == obj ) {
      *fp = f->next;
      free(f);
    } else {
      fp = f_next;
    }
  }
}

static
void rb_gc_invoke_all_finalizers_malloc()
{
  struct finalizer *f = finalizers;
  while ( f ) {
    void *f_next = f->next;
    fprintf(stderr, "\n  rb_gc_run_finalizer(%p, %p)\n", (void*) f->obj, (void*) f->block);
    rb_gc_run_finalizer(f->obj, f->block);
    free(f);
    f = f_next;
  }
}

static void rb_gc_at_exit_malloc()
{
  rb_gc_invoke_all_finalizers_malloc();
  if ( show_total_at_exit ) {
    fprintf(stderr, "\n  rb_gc_at_exit_malloc: pid=%d alloc_id=%lu total_size=%lu\n", (int) getpid(), (unsigned long) alloc_id, (unsigned long) total_size);
  }
}

static void mem_sys_malloc_options(rb_mem_sys *ms, const char *options)
{
  if ( (show_trace = ! ! strchr(options, 'D')) || 
       (show_total_at_exit = ! ! strchr(options, 'E')) ||
       0 ){
    ms->newobj = rb_newobj_malloc_debug;
  }
  show_line = ! ! strchr(options, 'L');
  show_register_address = ! ! strchr(options, 'R');
}

rb_mem_sys rb_mem_sys_malloc = {
  "malloc",
  0,
  0, /* initialize */
  mem_sys_malloc_options,
  0, /* Init_GC */
  rb_newobj_malloc,
  rb_gc_malloc,
  rb_gc_mark_malloc,
  rb_gc_mark_locations_malloc,
  rb_gc_markedQ_malloc,
  rb_gc_register_address_malloc,
  rb_gc_unregister_address_malloc,
  rb_gc_define_finalizer_malloc,
  rb_gc_undefine_finalizer_malloc,
  rb_gc_at_exit_malloc,
};

