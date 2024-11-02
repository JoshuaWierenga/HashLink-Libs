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
#include <hl.h>
#include <hlmodule.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef HL_WIN
#include <locale.h>
typedef uchar pchar;
#define pprintf(str,file)	uprintf(USTR(str),file)
#define pfopen(file,ext) _wfopen(file,USTR(ext))
#define pcompare wcscmp
#define ptoi(s)	wcstol(s,NULL,10)
#define PSTR(x) USTR(x)
#else
#include <sys/stat.h>
typedef char pchar;
#define pprintf printf
#define pfopen fopen
#define pcompare strcmp
#define ptoi atoi
#define PSTR(x) x
#endif


#ifndef USE_BYTECODE
extern void *hl_functions_ptrs[];
extern hl_type *hl_functions_types[];
void hl_init_hashes(void);
void hl_init_roots(void);
void hl_init_types(hl_module_context *ctx);
#endif

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

#ifndef USE_BYTECODE
static hl_type *find_fun_type(const void *fun) {
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
  if (!fun_type) {
    if (failed) {
      *failed = true;
    }
    return NULL;
  }

  return call(fun_type, fun, argc, argv, failed);
}
#endif

#ifdef USE_BYTECODE
static hl_code *code;
static hl_module *module;

static hl_code *load_code(const pchar *file, char **error_msg, bool print_errors) {
  hl_code *code;
  FILE *f = pfopen(file,"rb");
  int pos, size;
  char *fdata;
  if(f == NULL) {
    if(print_errors) {
      pprintf("File not found '%s'\n", file);
    }
    return NULL;
  }
  fseek(f, 0, SEEK_END);
  size = (int)ftell(f);
  fseek(f, 0, SEEK_SET);
  fdata = (char*)malloc(size);
  pos = 0;
  while(pos < size) {
    int r = (int)fread(fdata + pos, 1, size-pos, f);
    if(r <= 0) {
      if( print_errors ) {
        pprintf("Failed to read '%s'\n", file);
      }
      return NULL;
    }
    pos += r;
  }
  fclose(f);
  code = hl_code_read((unsigned char*)fdata, size, error_msg);
  free(fdata);
  return code;
}

// Idx for module->function_indexes and module->function_ptrs arrays
static int find_fun_idx(const uchar *requested_class, const uchar *requested_fun) {
  for (int i = 0; i < code->nfunctions; ++i) {
    int code_idx = module->functions_indexes[i];
    if (code_idx < 0 || code_idx >= code->nfunctions) {
      continue;
    }

    hl_type_obj *fun_type = fun_obj(&code->functions[code_idx]);
    if (!fun_type) {
      continue;
    }

    const uchar *actual_class = fun_type->name;
    const uchar *actual_fun = fun_field_name(&code->functions[code_idx]);

    // printf("Function %d: ", i);
    // uprintf(USTR("%s, "), actual_class);
    // uprintf(USTR("%s\n"), actual_fun);

    if (ucmp(requested_class, actual_class) == 0
        && ucmp(requested_fun, actual_fun) == 0) {
      return i;
    }
  }

  return -1;
}

vdynamic *hl_lib_call(const uchar *requested_class, const uchar *requested_fun,
                      int argc, vdynamic **argv, bool *failed) {
  int fun_idx = find_fun_idx(requested_class, requested_fun);
  if (-1 == fun_idx) {
    if (failed) {
      *failed = true;
    }
    return NULL;
  }

  hl_type *fun_type = code->functions[module->functions_indexes[fun_idx]].type;
  void *fun = module->functions_ptrs[fun_idx];

  return call(fun_type, fun, argc, argv, failed);
}
#endif

bool hl_lib_setup(int argc, char *argv[]) {
  vdynamic *ret;

  hl_global_init();
  hl_register_thread(&ret);
  hl_sys_init((void**)(argv + 1), argc - 1, NULL);

#ifdef USE_BYTECODE
  pchar *file = PSTR("PCTest.hl");
  FILE *fchk = pfopen(file,"rb");
  if(fchk == NULL) {
    pprintf("Unable to find bytecode file: %s\n", file);
    return false;
  }

  char *error_msg = NULL;
  code = load_code(file, &error_msg, true);
  if (!code) {
    if(error_msg) {
      printf("%s\n", error_msg);
    }
    return false;
  }

  module = hl_module_alloc(code);
  if (!module) {
    return false;
  }
  if(!hl_module_init(module, false, false)) {
    return false;
  }

  hl_code_free(code);
#else
  hl_setup_callbacks(hlc_static_call, hlc_get_wrapper);
#endif

  hl_type_fun tf = { 0 };
  hl_type clt = { 0 };
  bool failed;

  tf.ret = &hlt_void;
  clt.kind = HFUN;
  clt.fun = &tf;

#ifndef USE_BYTECODE
  call(&clt, hl_entry_point, 0, NULL, &failed);
#endif
  return !failed;
}

void hl_lib_cleanup(void) {
#ifdef USE_BYTECODE
  hl_module_free(module);
  hl_free(&code->alloc);
#endif
  hl_global_free();
}
