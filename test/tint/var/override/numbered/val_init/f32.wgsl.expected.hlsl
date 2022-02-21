#ifndef WGSL_SPEC_CONSTANT_1234
#define WGSL_SPEC_CONSTANT_1234 1.0f
#endif
static const float o = WGSL_SPEC_CONSTANT_1234;

[numthreads(1, 1, 1)]
void main() {
  return;
}
