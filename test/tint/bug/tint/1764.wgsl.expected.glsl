#version 310 es

shared int W[246];
void tint_symbol(uint local_invocation_index) {
  {
    for(uint idx = local_invocation_index; (idx < 246u); idx = (idx + 1u)) {
      uint i = idx;
      W[i] = 0;
    }
  }
  barrier();
  W[0] = 42;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
