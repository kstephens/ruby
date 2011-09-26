/**********************************************************************

  mem_sys_smal.h - SMAL GC memory system for Ruby.

  Author: Kurt Stephens
  created at: 2011/09/07

  Copyright (C) 2011 Kurt Stephens

**********************************************************************/
#include "ruby/ruby.h"
#include "ruby/st.h"
#include "ruby/re.h"
#include "ruby/io.h"
#include "ruby/util.h"
#include "eval_intern.h"
#include "vm_core.h"
#include "internal.h"
#include "gc.h"
#include "constant.h"

#include "mem_api.h"
#include <stdlib.h>
#include <assert.h>
#include "smal/smal.h"

typedef void *rb_objspace_t; 
static rb_objspace_t rb_objspace;
#include "rvalue.h"

#if 0
#define STATIC static
#include "gc_mark.c"
#undef STATIC

/* Based on gc_mark_children() in gc.c */
#define obj ((VALUE) _ptr)
#define ptr RANY(_ptr)
#define gc_mark(OS, REF, LEV) smal_mark_ptr(_ptr, (void*) (REF))
#define mark_locations_array(OS, X, N) smal_mark_ptr_n(_ptr, X, N)
#define objspace ((void*) 0)
#define lev 0

static inline void *base_mark(void *_ptr)
{
    if (FL_TEST(obj, FL_EXIVAR)) {
	rb_mark_generic_ivar(ptr);
    }
    gc_mark(objspace, obj->as.basic.klass, lev);
    return 0; // FIXME
}
static void *object_mark(void *_ptr)
{
  base_mark(_ptr);
  {
    long i, len = ROBJECT_NUMIV(obj);
    VALUE *pptr = ROBJECT_IVPTR(obj);
    for (i  = 0; i < len; i++) {
      gc_mark(objspace, *pptr++, lev);
    }
  }
  return 0; // FIXME
}
static void *class_mark(void *_ptr)
{
  base_mark(_ptr);
  mark_m_tbl(objspace, RCLASS_M_TBL(obj), lev);
  mark_tbl(objspace, RCLASS_IV_TBL(obj), lev);
  mark_const_tbl(objspace, RCLASS_CONST_TBL(obj), lev);
  return (void*) RCLASS_SUPER(obj);
}
static void *module_mark(void *_ptr)
{
  return base_mark(_ptr);
}
static void *float_mark(void *_ptr)
{
  return base_mark(_ptr);
}
static void *string_mark(void *_ptr)
{
  base_mark(_ptr);
#define STR_ASSOC FL_USER3   /* copied from string.c */
	if (FL_TEST(obj, RSTRING_NOEMBED) && FL_ANY(obj, ELTS_SHARED|STR_ASSOC)) {
	    return obj->as.string.as.heap.aux.shared;
	}
#undef STR_ASSOC
}
static void *regexp_mark(void *_ptr)
{
  base_mark(_ptr);
  return obj->as.regexp.src;
}
static void *array_mark(void *_ptr)
{
  base_mark(_ptr);
	if (FL_TEST(obj, ELTS_SHARED)) {
	    return obj->as.array.as.heap.aux.shared;
	}
	else {
	    long i, len = RARRAY_LEN(obj);
	    VALUE *ptr = RARRAY_PTR(obj);
	    for (i=0; i < len; i++) {
		gc_mark(objspace, *ptr++, lev);
	    }
	}
	return 0; // FIXME
}
static void *hash_mark(void *_ptr)
{
  base_mark(_ptr);
	mark_hash(objspace, obj->as.hash.ntbl, lev);
	return obj->as.hash.ifnone;
}
static void *struct_mark(void *_ptr)
{
  base_mark(_ptr);
  {
    long len = RSTRUCT_LEN(obj);
    VALUE *ptr = RSTRUCT_PTR(obj);
    
    while (len--) {
      gc_mark(objspace, *ptr++, lev);
    }
  }
}
static void *bignum_mark(void *_ptr)
{
  return base_mark(_ptr);
}
static void *file_mark(void *_ptr)
{
  base_mark(_ptr);
        if (obj->as.file.fptr) {
            gc_mark(objspace, obj->as.file.fptr->pathv, lev);
            gc_mark(objspace, obj->as.file.fptr->tied_io_for_writing, lev);
            gc_mark(objspace, obj->as.file.fptr->writeconv_asciicompat, lev);
            gc_mark(objspace, obj->as.file.fptr->writeconv_pre_ecopts, lev);
            gc_mark(objspace, obj->as.file.fptr->encs.ecopts, lev);
            gc_mark(objspace, obj->as.file.fptr->write_lock, lev);
        }
	return 0; // FIXME
}
static void *data_mark(void *_ptr)
{
  base_mark(_ptr);
	if (RTYPEDDATA_P(obj)) {
	    RUBY_DATA_FUNC mark_func = obj->as.typeddata.type->function.dmark;
	    if (mark_func) (*mark_func)(DATA_PTR(obj));
	}
	else {
	    if (obj->as.data.dmark) (*obj->as.data.dmark)(DATA_PTR(obj));
	}
	return 0;
}
static void *match_mark(void *_ptr)
{
  base_mark(_ptr);
  gc_mark(objspace, obj->as.match.regexp, lev);
  if (obj->as.match.str) {
    return obj->as.match.str;
  }
  return 0; // FIXME
}
static void *complex_mark(void *_ptr)
{
  base_mark(_ptr);
  gc_mark(objspace, obj->as.complex.real, lev);
  return obj->as.complex.imag;
}
static void *rational_mark(void *_ptr)
{
  base_mark(_ptr);
  gc_mark(objspace, obj->as.rational.num, lev);
  return obj->as.rational.den;
}
static void *node_mark(void *_ptr)
{
	switch (nd_type(obj)) {
	  case NODE_IF:		/* 1,2,3 */
	  case NODE_FOR:
	  case NODE_ITER:
	  case NODE_WHEN:
	  case NODE_MASGN:
	  case NODE_RESCUE:
	  case NODE_RESBODY:
	  case NODE_CLASS:
	  case NODE_BLOCK_PASS:
	    gc_mark(objspace, (VALUE)obj->as.node.u2.node, lev);
	    /* fall through */
	  case NODE_BLOCK:	/* 1,3 */
	  case NODE_OPTBLOCK:
	  case NODE_ARRAY:
	  case NODE_DSTR:
	  case NODE_DXSTR:
	  case NODE_DREGX:
	  case NODE_DREGX_ONCE:
	  case NODE_ENSURE:
	  case NODE_CALL:
	  case NODE_DEFS:
	  case NODE_OP_ASGN1:
	  case NODE_ARGS:
	    gc_mark(objspace, (VALUE)obj->as.node.u1.node, lev);
	    /* fall through */
	  case NODE_SUPER:	/* 3 */
	  case NODE_FCALL:
	  case NODE_DEFN:
	  case NODE_ARGS_AUX:
	    return (VALUE)obj->as.node.u3.node;

	  case NODE_WHILE:	/* 1,2 */
	  case NODE_UNTIL:
	  case NODE_AND:
	  case NODE_OR:
	  case NODE_CASE:
	  case NODE_SCLASS:
	  case NODE_DOT2:
	  case NODE_DOT3:
	  case NODE_FLIP2:
	  case NODE_FLIP3:
	  case NODE_MATCH2:
	  case NODE_MATCH3:
	  case NODE_OP_ASGN_OR:
	  case NODE_OP_ASGN_AND:
	  case NODE_MODULE:
	  case NODE_ALIAS:
	  case NODE_VALIAS:
	  case NODE_ARGSCAT:
	    gc_mark(objspace, (VALUE)obj->as.node.u1.node, lev);
	    /* fall through */
	  case NODE_GASGN:	/* 2 */
	  case NODE_LASGN:
	  case NODE_DASGN:
	  case NODE_DASGN_CURR:
	  case NODE_IASGN:
	  case NODE_IASGN2:
	  case NODE_CVASGN:
	  case NODE_COLON3:
	  case NODE_OPT_N:
	  case NODE_EVSTR:
	  case NODE_UNDEF:
	  case NODE_POSTEXE:
	    return (VALUE)obj->as.node.u2.node;

	  case NODE_HASH:	/* 1 */
	  case NODE_LIT:
	  case NODE_STR:
	  case NODE_XSTR:
	  case NODE_DEFINED:
	  case NODE_MATCH:
	  case NODE_RETURN:
	  case NODE_BREAK:
	  case NODE_NEXT:
	  case NODE_YIELD:
	  case NODE_COLON2:
	  case NODE_SPLAT:
	  case NODE_TO_ARY:
	    return (VALUE)obj->as.node.u1.node;

	  case NODE_SCOPE:	/* 2,3 */
	  case NODE_CDECL:
	  case NODE_OPT_ARG:
	    gc_mark(objspace, (VALUE)obj->as.node.u3.node, lev);
	    return (VALUE)obj->as.node.u2.node;

	  case NODE_ZARRAY:	/* - */
	  case NODE_ZSUPER:
	  case NODE_VCALL:
	  case NODE_GVAR:
	  case NODE_LVAR:
	  case NODE_DVAR:
	  case NODE_IVAR:
	  case NODE_CVAR:
	  case NODE_NTH_REF:
	  case NODE_BACK_REF:
	  case NODE_REDO:
	  case NODE_RETRY:
	  case NODE_SELF:
	  case NODE_NIL:
	  case NODE_TRUE:
	  case NODE_FALSE:
	  case NODE_ERRINFO:
	  case NODE_BLOCK_ARG:
	    break;
	  case NODE_ALLOCA:
	    mark_locations_array(objspace,
				 (VALUE*)obj->as.node.u1.value,
				 obj->as.node.u3.cnt);
	    return (VALUE)obj->as.node.u2.node;

	  default:		/* unlisted NODE */
	    // if (is_pointer_to_heap(objspace, obj->as.node.u1.node)) {
		gc_mark(objspace, (VALUE)obj->as.node.u1.node, lev);
		// }
		// if (is_pointer_to_heap(objspace, obj->as.node.u2.node)) {
		gc_mark(objspace, (VALUE)obj->as.node.u2.node, lev);
		// }
		// if (is_pointer_to_heap(objspace, obj->as.node.u3.node)) {
		return (VALUE)obj->as.node.u3.node;
		// }
	}
	return 0;
}

#undef obj
#undef gc_mark

#endif

static struct rb_smal_type {
  smal_type *st;
  int rt; /* enum ruby_value_type */ 
  size_t size;
  void *mark_func;
  void *free_func;
} type_map[] = {
  { 0, RUBY_T_NONE, 0 },
  { 0, RUBY_T_OBJECT,   sizeof(struct RObject), 0 }, // object_mark },
  { 0, RUBY_T_CLASS,    sizeof(struct RClass), 0 }, // class_mark },
  { 0, RUBY_T_MODULE,   sizeof(struct RClass), 0 }, // class_mark }, // ???
  { 0, RUBY_T_FLOAT,    sizeof(struct RFloat), 0 }, // float_mark },
  { 0, RUBY_T_STRING,   sizeof(struct RString), 0 }, // string_mark },
  { 0, RUBY_T_REGEXP,   sizeof(struct RRegexp), 0 }, // regexp_mark },
  { 0, RUBY_T_ARRAY,    sizeof(struct RArray), 0 }, // array_mark },
  { 0, RUBY_T_HASH,     sizeof(struct RHash), 0 }, // hash_mark },
  { 0, RUBY_T_STRUCT,   sizeof(struct RStruct), 0 }, // struct_mark },
  { 0, RUBY_T_BIGNUM,   sizeof(struct RBignum), 0 }, // bignum_mark },
  { 0, RUBY_T_FILE,     sizeof(struct RFile), 0 }, // file_mark },
  { 0, RUBY_T_DATA,     sizeof(struct RData), 0 }, // data_mark }, // ???? RTypedData
  { 0, RUBY_T_MATCH,    sizeof(struct RMatch), 0 }, // match_mark},
  { 0, RUBY_T_COMPLEX,  sizeof(struct RComplex), 0 }, // complex_mark },
  { 0, RUBY_T_RATIONAL, sizeof(struct RRational), 0 }, // rational_mark },

  { 0, RUBY_T_NIL,      0 },
  { 0, RUBY_T_TRUE,     0 },
  { 0, RUBY_T_FALSE,    0 },
  { 0, RUBY_T_SYMBOL,   0 },
  { 0, RUBY_T_FIXNUM,   0 },

  { 0, RUBY_T_UNDEF,    0 }, 
  { 0, RUBY_T_NODE,     sizeof(struct RNode), 0 }, // node_mark, }, 
  { 0, RUBY_T_ICLASS,   sizeof(struct RClass), 0 }, // class_mark }, // ???? 
  { 0, RUBY_T_ZOMBIE,   0 },
  { 0, RUBY_T_MASK,     0 },
  { 0, RUBY_T_MASK + 1, 0 }, // GENERIC RVALUE (see gc.c)

  { 0, -1, 0 },
};

static int    til_gc = 16000;
static int    til_gc_d0 = 16000;
static int    til_gc_max = 1000000;
static double til_gc_d1 = 2.5;
static double til_gc_d2 = 0.5;

static VALUE rb_newobj__smal(enum ruby_value_type type)
{
  struct rb_smal_type *t = &type_map[type];
  VALUE value;

#if 0
  /* GC after allocation count down: Ugly metric. */
  if ( -- til_gc < 0 ) {
    int til_gc_d0_last = til_gc_d0;
    til_gc_d0 += til_gc_d0 * til_gc_d1;
    til_gc_d1 *= til_gc_d2;
    if ( til_gc_d0 <= 0 ) til_gc_d0 = til_gc_d0_last;
    if ( til_gc_d0 > til_gc_max ) til_gc_d0 = til_gc_max;
    til_gc = til_gc_d0;
    smal_collect();
  }
#endif

  value = (VALUE) smal_alloc(t->st);

  return value;
}

static VALUE rb_newobj_smal()
{
  rb_newobj__smal(RUBY_T_MASK + 1);
}

static void rb_gc_smal()
{
  smal_collect();
}

static void rb_gc_mark_smal(VALUE object)
{
  smal_mark_ptr(0, (void*) object);
}

static void rb_gc_mark_locations_smal(VALUE *start, VALUE *end)
{
  smal_mark_ptr_range(0, start, end);
}

static int  rb_gc_markedQ_smal(VALUE object)
{
  return smal_object_reachableQ((void*) object);
}

static struct root {
  VALUE *addr;
  struct root *next;
} *roots; /* NOT NATIVE THREAD-SAFE */

static void rb_gc_register_address_smal(VALUE *addr)
{
  struct root *r = malloc(sizeof(*r));
  r->addr = addr;
  r->next = roots;
  roots = r;
}

static void rb_gc_unregister_address_smal(VALUE *addr)
{
  struct root **rp = &roots;
  while ( *rp ) {
    if ( (*rp)->addr == addr ) {
      struct root *r = *rp;
      *rp = r->next;
      free(r);
      break;
    }
    rp = &(*rp)->next;
  }
}

static void rb_gc_define_finalizer_smal(VALUE obj, VALUE proc)
{
}

static void rb_gc_undefine_finalizer_smal(VALUE obj)
{
}


static void rb_gc_at_exit_smal()
{
}

static void Init_GC_smal()
{
  smal_init();
  {
    int i;
    struct rb_smal_type *t;
    for ( i = 0; (t = &type_map[i])->rt >=0; ++ i ) {
      smal_type_descriptor desc;
      assert(i == t->rt);
      desc.mark_func = t->mark_func;
      desc.free_func = t->free_func;
      if ( t->rt == RUBY_T_MASK + 1 ) {
	t->size = rb_sizeof_RVALUE; // HACK!
      }
      if ( t->size ) 
	t->st = smal_type_for_desc(&desc);
    }
  }
}


rb_mem_sys rb_mem_sys_smal = {
  "smal",
  0,
  0, /* initialize */
  0, /* options */
  Init_GC_smal, /* Init_GC */
  rb_newobj_smal,
  rb_gc_smal,
  rb_gc_mark_smal,
  rb_gc_mark_locations_smal,
  rb_gc_markedQ_smal,
  rb_gc_register_address_smal,
  rb_gc_unregister_address_smal,
  rb_gc_define_finalizer_smal,
  rb_gc_undefine_finalizer_smal,
  rb_gc_at_exit_smal,
};

/*********************************/

void smal_collect_before_inner(void *top_of_stack)
{
}

void smal_collect_before_mark()
{
}

void smal_collect_mark_roots()
{
  struct root *r;
  for ( r = roots; r; r = r->next ) {
    smal_mark_ptr(0, (void*) *(r->addr));
  }
}

void smal_collect_after_mark()
{
}

void smal_collect_before_sweep()
{
}

void smal_collect_after_sweep()
{
}

