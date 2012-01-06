/**********************************************************************

  mem_api.c - Memory/GC internals API.

  Author: Kurt Stephens

  Copyright (C) 2010, 2011 Kurt Stephens
*/

#include "ruby.h"
#include "mem_api.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

static rb_mem_sys ms; /* The active rb_mem_sys. */
static rb_mem_sys *selected; /* The selected rb_mem_sys, copied to ms. */
static rb_mem_sys *mem_sys_list = 0; /* List of registered rb_mem_sys objects. */

void Init_heap_core(); /* gc.c */

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
  MEM_SYS(malloc);
  MEM_SYS(core);
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
      selected = p;
      ms = *p;
      ms.opts = options;
      if ( ms.initialize )
	ms.initialize(&ms);
      if ( ms.options && *options )
	ms.options(&ms, options);
      // fprintf(stderr, "\nrb_mem_sys_select: pid=%d selected %s\n", (int) getpid(), p->name);
    } else {
      rb_fatal("rb_mem_sys_select: cannot locate %s", name);
      abort();
    }
  }
}

static void rb_mem_sys_event_log_open(const char *event_log_file);

void Init_mem_sys()
{
  static int initialized = 0;
  if ( initialized ) return;
  ++ initialized;
  rb_mem_sys_init();
  rb_mem_sys_select(0);
  rb_mem_sys_event_log_open(0);
}

void Init_heap()
{
  (ms.Init_heap ? ms.Init_heap : Init_heap_core)();
}

/********************************************************************
 * Internal interface to rb_mem_sys methods.
 */

void *ruby_xmalloc(size_t size)
{
  return ms.ruby_xmalloc(size);
}

void ruby_xfree(void *ptr)
{
  ms.ruby_xfree(ptr);
}

void *ruby_xrealloc(void *ptr, size_t size)
{
  return ms.ruby_xrealloc(ptr, size);
}

void *ruby_xcalloc(size_t size1, size_t size2)
{
  return ms.ruby_xcalloc(size1, size2);
}

VALUE rb_newobj(void)
{
  VALUE obj;
  if ( RB_MEM_SYS_TRACE_EVENTS > 1 ) {
    rb_mem_sys_invoke_callbacks(RB_MEM_SYS_EVENT_OBJECT_ALLOC,
				RB_MEM_SYS_EVENT_BEFORE,
				0, rb_sizeof_RVALUE);
  }
  obj = ms.newobj();
#ifdef GC_DEBUG
  RANY(obj)->file = rb_sourcefile();
  RANY(obj)->line = rb_sourceline();
#endif
  if ( RB_MEM_SYS_TRACE_EVENTS ) {
    rb_mem_sys_invoke_callbacks(RB_MEM_SYS_EVENT_OBJECT_ALLOC,
				RB_MEM_SYS_EVENT_AFTER,
				(void*) obj, rb_sizeof_RVALUE);
  }
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
  if ( RB_MEM_SYS_TRACE_EVENTS ) {
    rb_mem_sys_invoke_callbacks(RB_MEM_SYS_EVENT_FINALIZER_ALLOC,
				RB_MEM_SYS_EVENT_AFTER,
				(void*) obj, 0);
  }
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
  rb_mem_sys_invoke_callbacks(RB_MEM_SYS_EVENT_AT_EXIT,
			      RB_MEM_SYS_EVENT_AFTER,
			      0, 0);
}

/* See eval.c */
void rb_gc_call_finalizer_at_exit(void)
{
  rb_gc_at_exit(); 
}

/********************************************************************/

typedef struct rb_mem_sys_callback {
  struct rb_mem_sys_callback *next, *prev;
  enum rb_mem_sys_event event;
  enum rb_mem_sys_event_location location;
  void (*func)(void *, void*, void*, size_t);
  void *func_data;
} rb_mem_sys_callback;

static rb_mem_sys_callback callbacks[rb_mem_sys_event__LAST][rb_mem_sys_event_location__LAST];

void *rb_mem_sys_add_callback(enum rb_mem_sys_event event, 
			      enum rb_mem_sys_event_location location, 
			      void (*func)(void *callback, void *func_data, void *addr, size_t size), 
			      void *func_data)
{
  rb_mem_sys_callback *cb = malloc(sizeof(*cb));
  cb->event = event;
  cb->location = location;
  cb->func = func;
  cb->func_data = func_data;
  {
    rb_mem_sys_callback *pos = &callbacks[event][location];
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

void rb_mem_sys_remove_callback(void *callback)
{
  rb_mem_sys_callback *cb = callback;
  cb->func = 0; /* guard. */
  cb->next->prev = cb->prev;
  cb->prev->next = cb->next;
  free(cb);
}

void rb_mem_sys_set_callback_func(void *callback, void *func)
{
  rb_mem_sys_callback *cb = callback;
  cb->func = func;
}

void rb_mem_sys_invoke_callbacks(enum rb_mem_sys_event event, 
				 enum rb_mem_sys_event_location location,
				 void *addr,
				 size_t size)
{
  rb_mem_sys_callback *end = &callbacks[event][location]; 
  rb_mem_sys_callback *cb = end->next;
  int zero_words[128];
  /* Uninitialized and empty linked list head. */
  if ( cb && cb != end ) {
    int func_called = 0;
    do {
      /* Callback func may invoke rb_gc_remove_callback() on itself. */
      rb_mem_sys_callback *cb_next = cb->next; 
      if ( cb->func ) {
	func_called = 1;
	cb->func(cb, cb->func_data, addr, size);
      }
      cb = cb_next;
    } while ( cb != end );
    /* Avoid garbage on stack. */
    if ( func_called ) 
      memset(zero_words, 0, sizeof(zero_words));
  }
}

/********************************************************************/

static unsigned long object_alloc_id;
static unsigned long event_id;
static FILE *event_log;
static const char *event_log_file;

static void event_log_object_alloc(void *callback, void *func_data, void *addr, size_t size)
{
  if ( ! event_log ) return;
  ++ object_alloc_id;
  ++ event_id;
  fprintf(event_log, "%d %lu %lu oa %p %lu\n", (int) getpid(), event_id, object_alloc_id, addr, size);
}
 
static void event_log_object_free(void *callback, void *func_data, void *addr, size_t size)
{
  if ( ! event_log ) return;
  ++ event_id;
  fprintf(event_log, "%d %lu %lu of %p %lu\n", (int) getpid(), event_id, object_alloc_id, addr, size);
}

static void event_log_page_alloc(void *callback, void *func_data, void *addr, size_t size)
{
  if ( ! event_log ) return;
  ++ event_id;
  fprintf(event_log, "%d %lu %lu pa %p %lu\n", (int) getpid(), event_id, object_alloc_id, addr, size);
}
 
static void event_log_page_free(void *callback, void *func_data, void *addr, size_t size)
{
  if ( ! event_log ) return;
  ++ event_id;
  fprintf(event_log, "%d %lu %lu pf %p %lu\n", (int) getpid(), event_id, object_alloc_id, addr, size);
}

static void event_log_finalizer_alloc(void *callback, void *func_data, void *addr, size_t size)
{
  if ( ! event_log ) return;
  ++ event_id;
  fprintf(event_log, "%d %lu %lu fa %p %lu\n", (int) getpid(), event_id, object_alloc_id, addr, size);
}
 
static void event_log_finalizer_free(void *callback, void *func_data, void *addr, size_t size)
{
  if ( ! event_log ) return;
  ++ event_id;
  fprintf(event_log, "%d %lu %lu ff %p %lu\n", (int) getpid(), event_id, object_alloc_id, addr, size);
}

static void event_log_close()
{
  if ( ! event_log ) return;
  if ( event_log != stderr ) fclose(event_log);
  event_log = 0;
}

static void event_log_at_exit(void *callback, void *func_data, void *addr, size_t size)
{
  if ( ! event_log ) return;
  ++ event_id;
  fprintf(event_log, "%d %lu %lu EXIT\n", (int) getpid(), event_id, object_alloc_id);
  event_log_close();
}

static void rb_mem_sys_add_event_log_hooks()
{
  static int done;
  if ( done ) return;
  rb_mem_sys_add_callback(RB_MEM_SYS_EVENT_OBJECT_ALLOC, RB_MEM_SYS_EVENT_AFTER, event_log_object_alloc, 0);
  rb_mem_sys_add_callback(RB_MEM_SYS_EVENT_OBJECT_FREE,  RB_MEM_SYS_EVENT_AFTER, event_log_object_free, 0);
  rb_mem_sys_add_callback(RB_MEM_SYS_EVENT_PAGE_ALLOC,   RB_MEM_SYS_EVENT_AFTER, event_log_page_alloc, 0);
  rb_mem_sys_add_callback(RB_MEM_SYS_EVENT_PAGE_FREE,    RB_MEM_SYS_EVENT_AFTER, event_log_page_free, 0);
  rb_mem_sys_add_callback(RB_MEM_SYS_EVENT_FINALIZER_ALLOC,   RB_MEM_SYS_EVENT_AFTER, event_log_finalizer_alloc, 0);
  rb_mem_sys_add_callback(RB_MEM_SYS_EVENT_FINALIZER_FREE,    RB_MEM_SYS_EVENT_AFTER, event_log_finalizer_free, 0);
  rb_mem_sys_add_callback(RB_MEM_SYS_EVENT_AT_EXIT,      RB_MEM_SYS_EVENT_AFTER, event_log_at_exit, 0);
  done = 1;
}

static void rb_mem_sys_event_log_open(const char *file)
{
  event_log_file = file;
  if ( ! (event_log_file && *event_log_file) ) 
    event_log_file = getenv("RUBY_MEM_SYS_EVENT_LOG");
  if ( (event_log_file && *event_log_file) ) {
    if ( ! strcmp(event_log_file, "STDERR") ) {
      event_log = stderr;
    } else {
      event_log = fopen(event_log_file, "a+");
    }
    if ( ! event_log ) {
      fprintf(stderr, "ruby: Cannot open RUBY_MEM_SYS_EVENT_LOG=%s\n", event_log_file);
      return;
    }
    rb_mem_sys_add_event_log_hooks();
  }
}


/********************************************************************
 * Ruby interface to mem_sys state.
 */

static VALUE rb_mMemSys;

static VALUE rb_mem_sys_name(void)
{
  assert(selected);
  assert(selected->name);
  return rb_str_new_cstr(selected->name);
}

static VALUE rb_mem_sys_opts(void)
{
  assert(ms.opts);
  return rb_str_new_cstr(ms.opts);
}

static VALUE rb_mem_sys_supported(void)
{
  VALUE result = rb_ary_new();
  rb_mem_sys *p = mem_sys_list;
  while ( p ) {
    rb_ary_push(result, rb_str_new_cstr(p->name));
    p = p->next;
  }
  return result;
}

static VALUE rb_mem_sys_event_log(void)
{
  return event_log_file ? rb_str_new_cstr(event_log_file) : Qnil;
}

static VALUE rb_mem_sys_argv0(void) /* HACK */
{
  return rb_argv0;
}

void Init_mem_sys_methods()
{
  rb_mMemSys = rb_define_module_under(rb_mGC, "MemSys");
  rb_define_singleton_method(rb_mMemSys, "name", rb_mem_sys_name, 0);
  rb_define_singleton_method(rb_mMemSys, "opts", rb_mem_sys_opts, 0);
  rb_define_singleton_method(rb_mMemSys, "supported", rb_mem_sys_supported, 0);
  rb_define_singleton_method(rb_mMemSys, "event_log", rb_mem_sys_event_log, 0);
  rb_define_singleton_method(rb_mMemSys, "argv0", rb_mem_sys_argv0, 0); /* HACK */
}

