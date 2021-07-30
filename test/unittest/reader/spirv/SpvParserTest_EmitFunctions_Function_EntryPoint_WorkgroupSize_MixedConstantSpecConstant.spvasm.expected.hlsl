#ifndef WGSL_SPEC_CONSTANT_0
#define WGSL_SPEC_CONSTANT_0 3u
#endif
static const uint x_3 = WGSL_SPEC_CONSTANT_0;
#ifndef WGSL_SPEC_CONSTANT_2
#define WGSL_SPEC_CONSTANT_2 7u
#endif
static const uint x_4 = WGSL_SPEC_CONSTANT_2;

void comp_main_1() {
  return;
}

[numthreads(3, 5, 7)]
void comp_main() {
  comp_main_1();
  return;
}
