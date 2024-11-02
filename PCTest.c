#include <hl.h>
#include <_std/String.h>

extern bool hl_lib_setup(int argc, char *argv[]);
extern vdynamic *hl_lib_call(void *fun, int argc, vdynamic **argv, bool *failed);
extern void hl_lib_cleanup(void);

extern String PCTest_test(void);

int main(int argc, char *argv[]) {
  if (!hl_lib_setup(argc, argv)) {
    return 1;
  }

  vdynamic *ret = hl_lib_call(PCTest_test, 0, NULL, NULL);
  uprintf(USTR("Test: %s\n"), hl_to_string(ret));

  hl_lib_cleanup();
  return 0;
}
