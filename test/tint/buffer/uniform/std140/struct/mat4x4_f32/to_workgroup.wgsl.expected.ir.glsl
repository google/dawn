#version 310 es


struct S {
  int before;
  mat4 m;
  int after;
};

layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  S tint_symbol[4];
} v;
shared S w[4];
void f_inner(uint tint_local_index) {
  {
    uint v_1 = 0u;
    v_1 = tint_local_index;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 4u)) {
        break;
      }
      w[v_2] = S(0, mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f)), 0);
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
  barrier();
  w = v.tint_symbol;
  w[1] = v.tint_symbol[2];
  w[3].m = v.tint_symbol[2].m;
  w[1].m[0] = v.tint_symbol[0].m[1].ywxz;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}
