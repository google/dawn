#version 310 es

shared int i;
void tint_symbol(uint local_invocation_index) {
  {
    i = 0;
  }
  barrier();
  i = 123;
  int u = (i + 1);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
