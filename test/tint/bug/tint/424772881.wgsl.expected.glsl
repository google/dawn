#version 310 es

layout(binding = 0, std430)
buffer result_block_1_ssbo {
  uvec3 inner;
} v_1;
shared bvec3 wgvar;
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    wgvar = bvec3(false);
  }
  barrier();
  bvec3 v = wgvar;
  wgvar = v;
  bool e = wgvar.x;
  wgvar.y = e;
  barrier();
  v_1.inner = uvec3(wgvar);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
