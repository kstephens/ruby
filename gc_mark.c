struct mark_tbl_arg {
    rb_objspace_t *objspace;
    int lev;
};

static int
mark_entry(st_data_t key, st_data_t value, st_data_t data)
{
    struct mark_tbl_arg *arg = (void*)data;
    gc_mark(arg->objspace, (VALUE)value, arg->lev);
    return ST_CONTINUE;
}

static void
mark_tbl(rb_objspace_t *objspace, st_table *tbl, int lev)
{
    struct mark_tbl_arg arg;
    if (!tbl || tbl->num_entries == 0) return;
    arg.objspace = objspace;
    arg.lev = lev;
    st_foreach(tbl, mark_entry, (st_data_t)&arg);
}

static int
mark_key(st_data_t key, st_data_t value, st_data_t data)
{
    struct mark_tbl_arg *arg = (void*)data;
    gc_mark(arg->objspace, (VALUE)key, arg->lev);
    return ST_CONTINUE;
}

static void
mark_set(rb_objspace_t *objspace, st_table *tbl, int lev)
{
    struct mark_tbl_arg arg;
    if (!tbl) return;
    arg.objspace = objspace;
    arg.lev = lev;
    st_foreach(tbl, mark_key, (st_data_t)&arg);
}

STATIC
void
rb_mark_set(st_table *tbl)
{
    mark_set(&rb_objspace, tbl, 0);
}

static int
mark_keyvalue(st_data_t key, st_data_t value, st_data_t data)
{
    struct mark_tbl_arg *arg = (void*)data;
    gc_mark(arg->objspace, (VALUE)key, arg->lev);
    gc_mark(arg->objspace, (VALUE)value, arg->lev);
    return ST_CONTINUE;
}

static void
mark_hash(rb_objspace_t *objspace, st_table *tbl, int lev)
{
    struct mark_tbl_arg arg;
    if (!tbl) return;
    arg.objspace = objspace;
    arg.lev = lev;
    st_foreach(tbl, mark_keyvalue, (st_data_t)&arg);
}

STATIC
void
rb_mark_hash(st_table *tbl)
{
    mark_hash(&rb_objspace, tbl, 0);
}

static void
mark_method_entry(rb_objspace_t *objspace, const rb_method_entry_t *me, int lev)
{
    const rb_method_definition_t *def = me->def;

    gc_mark(objspace, me->klass, lev);
    if (!def) return;
    switch (def->type) {
      case VM_METHOD_TYPE_ISEQ:
	gc_mark(objspace, def->body.iseq->self, lev);
	break;
      case VM_METHOD_TYPE_BMETHOD:
	gc_mark(objspace, def->body.proc, lev);
	break;
      case VM_METHOD_TYPE_ATTRSET:
      case VM_METHOD_TYPE_IVAR:
	gc_mark(objspace, def->body.attr.location, lev);
	break;
      default:
	break; /* ignore */
    }
}

STATIC
void
rb_mark_method_entry(const rb_method_entry_t *me)
{
    mark_method_entry(&rb_objspace, me, 0);
}

static int
mark_method_entry_i(ID key, const rb_method_entry_t *me, st_data_t data)
{
    struct mark_tbl_arg *arg = (void*)data;
    mark_method_entry(arg->objspace, me, arg->lev);
    return ST_CONTINUE;
}

static void
mark_m_tbl(rb_objspace_t *objspace, st_table *tbl, int lev)
{
    struct mark_tbl_arg arg;
    if (!tbl) return;
    arg.objspace = objspace;
    arg.lev = lev;
    st_foreach(tbl, mark_method_entry_i, (st_data_t)&arg);
}

