#version 310 es
precision mediump float;

ivec2 tint_atomicCompareExchangeWeak(inout int param_0, int param_1, int param_2) {
  ivec2 result;
  result.x = atomicCompSwap(param_0, param_1, param_2);
  result.y = result.x == param_2 ? 1 : 0;
  return result;
}



layout (binding = 0) buffer SB_RW_1 {
  int arg_0;
} sb_rw;

void atomicCompareExchangeWeak_12871c() {
  ivec2 res = tint_atomicCompareExchangeWeak(sb_rw.arg_0, 1, 1);
}

void fragment_main() {
  atomicCompareExchangeWeak_12871c();
  return;
}
void main() {
  fragment_main();
}


#version 310 es
precision mediump float;

ivec2 tint_atomicCompareExchangeWeak(inout int param_0, int param_1, int param_2) {
  ivec2 result;
  result.x = atomicCompSwap(param_0, param_1, param_2);
  result.y = result.x == param_2 ? 1 : 0;
  return result;
}



layout (binding = 0) buffer SB_RW_1 {
  int arg_0;
} sb_rw;

void atomicCompareExchangeWeak_12871c() {
  ivec2 res = tint_atomicCompareExchangeWeak(sb_rw.arg_0, 1, 1);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  atomicCompareExchangeWeak_12871c();
  return;
}
void main() {
  compute_main();
}


