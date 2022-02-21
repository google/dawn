#ifndef WGSL_SPEC_CONSTANT_1234
#define WGSL_SPEC_CONSTANT_1234 false
#endif
static const bool o = WGSL_SPEC_CONSTANT_1234;

[numthreads(1, 1, 1)]
void main() {
  return;
}
