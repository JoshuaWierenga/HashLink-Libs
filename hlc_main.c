/*
 * Copyright (C)2015-2016 Haxe Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

extern void *hl_functions_ptrs[];
extern hl_type *hl_functions_types[];
void hl_init_hashes(void);
void hl_init_roots(void);
void hl_init_types(hl_module_context *ctx);

static vdynamic *call(hl_type *fun_type, void *fun, int argc, vdynamic **argv, bool *failed) {
  vclosure cl = { 0 };
  vdynamic *ret;
  bool failed2;

  if (!failed) {
    failed = &failed2;
  }

  cl.t = fun_type;
  cl.fun = fun;

  ret = hl_dyn_call_safe(&cl, argv, argc, failed);
  if(*failed) {
    varray *a = hl_exception_stack();
    uprintf(USTR("Uncaught exception: %s\n"), hl_to_string(ret));
    for (int i = 0; i < a->size; ++i) {
      uprintf(USTR("Called from %s\n"), hl_aptr(a, uchar *)[i]);
    }
  }

  return ret;
}

static hl_type *find_fun_type(void *fun) {
  void **fun_ptr = hl_functions_ptrs;
  hl_type **fun_type = hl_functions_types;

  while (*fun_ptr != (void *)fun$init) {
    if (*fun_ptr == fun) {
      return *fun_type;
    }

    ++fun_ptr;
    ++fun_type;
  }

  return NULL;
}

vdynamic *hl_lib_call(void *fun, int argc, vdynamic **argv, bool *failed) {
  hl_type *fun_type = find_fun_type(fun);
  return call(fun_type, fun, argc, argv, failed);
}


bool hl_lib_setup(int argc, char *argv[]) {
  vdynamic *ret;

  hl_global_init();
  hl_register_thread(&ret);
  hl_sys_init((void**)(argv + 1), argc - 1, NULL);
  hl_setup_callbacks(hlc_static_call, hlc_get_wrapper);

  hl_type_fun tf = { 0 };
  hl_type clt = { 0 };
  bool failed;

  tf.ret = &hlt_void;
  clt.kind = HFUN;
  clt.fun = &tf;

  call(&clt, hl_entry_point, 0, NULL, &failed);
  return !failed;
}

void hl_lib_cleanup(void) {
  hl_global_free();
}
