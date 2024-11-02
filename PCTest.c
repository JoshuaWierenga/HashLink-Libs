#include <hl.h>
#include <_std/String.h>

extern bool hl_lib_setup(int argc, char *argv[]);
#ifdef USE_BYTECODE
vdynamic *hl_lib_call(const uchar *requested_class, const uchar *requested_fun,
                      int argc, vdynamic **argv, bool *failed);
#else
extern vdynamic *hl_lib_call(void *fun, int argc, vdynamic **argv, bool *failed);
#endif
extern void hl_lib_cleanup(void);

#ifndef USE_BYTECODE
extern String PCTest_test(void);
extern String PCTest2_test(void);
#endif

int main(int argc, char *argv[]) {
  if (!hl_lib_setup(argc, argv)) {
    return 1;
  }

#ifdef USE_BYTECODE
  vdynamic *ret = hl_lib_call(USTR("$PCTest"), USTR("test"), 0, NULL, NULL);
#else
  vdynamic *ret = hl_lib_call(PCTest_test, 0, NULL, NULL);
#endif
  uprintf(USTR("Test: %s\n"), hl_to_string(ret));

#ifdef USE_BYTECODE
  ret = hl_lib_call(USTR("$PCTest2"), USTR("test"), 0, NULL, NULL);
#else
  ret = hl_lib_call(PCTest2_test, 0, NULL, NULL);
#endif
  uprintf(USTR("Test: %s\n"), hl_to_string(ret));

  hl_lib_cleanup();
  return 0;
}
