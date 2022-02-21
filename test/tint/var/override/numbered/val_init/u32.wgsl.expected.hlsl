#ifndef WGSL_SPEC_CONSTANT_1234
#define WGSL_SPEC_CONSTANT_1234 1u
#endif
static const uint o = WGSL_SPEC_CONSTANT_1234;

[numthreads(1, 1, 1)]
void main() {
  return;
}
