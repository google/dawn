#version 310 es

shared int S;
int func_S() {
  return S;
}

void tint_symbol(uint local_invocation_index) {
  {
    S = 0;
  }
  barrier();
  int r = func_S();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
