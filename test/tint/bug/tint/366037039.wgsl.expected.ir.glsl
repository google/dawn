#version 310 es


struct S {
  uvec3 a;
  uint b;
  uvec3 c[4];
};

layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  S tint_symbol;
} v;
layout(binding = 1, std430)
buffer tint_symbol_3_1_ssbo {
  S tint_symbol_2;
} v_1;
shared S wbuffer;
void tint_store_and_preserve_padding_1(inout uvec3 target[4], uvec3 value_param[4]) {
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 4u)) {
        break;
      }
      target[v_3] = value_param[v_3];
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
}
void tint_store_and_preserve_padding(inout S target, S value_param) {
  target.a = value_param.a;
  target.b = value_param.b;
  tint_store_and_preserve_padding_1(target.c, value_param.c);
}
void foo() {
  S u = v.tint_symbol;
  S s = v_1.tint_symbol_2;
  S w = v_1.tint_symbol_2;
  tint_store_and_preserve_padding(v_1.tint_symbol_2, S(uvec3(0u), 0u, uvec3[4](uvec3(0u), uvec3(0u), uvec3(0u), uvec3(0u))));
  wbuffer = S(uvec3(0u), 0u, uvec3[4](uvec3(0u), uvec3(0u), uvec3(0u), uvec3(0u)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
