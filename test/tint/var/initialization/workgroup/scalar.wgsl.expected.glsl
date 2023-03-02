#version 310 es

shared int v;
void tint_symbol(uint local_invocation_index) {
  {
    v = 0;
  }
  barrier();
  int i = v;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
