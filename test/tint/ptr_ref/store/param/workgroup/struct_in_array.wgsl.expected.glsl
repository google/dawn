#version 310 es


struct str {
  int i;
};

shared str S[4];
void func(uint pointer_indices[1]) {
  S[pointer_indices[0u]] = str(0);
}
void main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v = idx;
      if ((v >= 4u)) {
        break;
      }
      S[v] = str(0);
      {
        idx = (idx + 1u);
      }
    }
  }
  barrier();
  func(uint[1](2u));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
