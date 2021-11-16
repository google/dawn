SKIP: FAILED

#version 310 es
precision mediump float;

groupshared int i;

struct tint_symbol_2 {
  uint local_invocation_index;
};

void tint_symbol_inner(uint local_invocation_index) {
  {
    i = 0;
  }
  GroupMemoryBarrierWithGroupSync();
  i = 123;
  int use = (i + 1);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol(tint_symbol_2 tint_symbol_1) {
  tint_symbol_inner(tint_symbol_1.local_invocation_index);
  return;
}
void main() {
  tint_symbol_2 inputs;
  inputs.local_invocation_index = uint(gl_LocalInvocationIndex);
  tint_symbol(inputs);
}


Error parsing GLSL shader:
ERROR: 0:4: '' :  syntax error, unexpected IDENTIFIER
ERROR: 1 compilation errors.  No code generated.



