// RUN: %llvmgcc -S %s -o - | llvm-as -o /dev/null
// Don't crash on a common-linkage constant global.
extern const int kABSourceTypeProperty;
int foo(void) {
  return kABSourceTypeProperty;
}
const int kABSourceTypeProperty;
