#version 310 es


struct S {
  uvec3 a;
  uint b;
  uvec3 c[4];
};

layout(binding = 0, std140)
uniform ubuffer_block_1_ubo {
  uvec4 inner[5];
} v;
layout(binding = 1, std430)
buffer sbuffer_block_1_ssbo {
  S inner;
} v_1;
shared S wbuffer;
void tint_store_and_preserve_padding_1(uvec3 value_param[4]) {
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 4u)) {
        break;
      }
      v_1.inner.c[v_3] = value_param[v_3];
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
}
void tint_store_and_preserve_padding(S value_param) {
  v_1.inner.a = value_param.a;
  v_1.inner.b = value_param.b;
  tint_store_and_preserve_padding_1(value_param.c);
}
uvec3[4] v_4(uint start_byte_offset) {
  uvec3 a[4] = uvec3[4](uvec3(0u), uvec3(0u), uvec3(0u), uvec3(0u));
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      a[v_6] = v.inner[((start_byte_offset + (v_6 * 16u)) / 16u)].xyz;
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
  return a;
}
S v_7(uint start_byte_offset) {
  uvec3 v_8 = v.inner[(start_byte_offset / 16u)].xyz;
  uvec4 v_9 = v.inner[((12u + start_byte_offset) / 16u)];
  return S(v_8, v_9[(((12u + start_byte_offset) & 15u) >> 2u)], v_4((16u + start_byte_offset)));
}
void foo_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    wbuffer.a = uvec3(0u);
    wbuffer.b = 0u;
  }
  {
    uint v_10 = 0u;
    v_10 = tint_local_index;
    while(true) {
      uint v_11 = v_10;
      if ((v_11 >= 4u)) {
        break;
      }
      wbuffer.c[v_11] = uvec3(0u);
      {
        v_10 = (v_11 + 1u);
      }
      continue;
    }
  }
  barrier();
  S u = v_7(0u);
  S s = v_1.inner;
  S w = v_1.inner;
  tint_store_and_preserve_padding(S(uvec3(0u), 0u, uvec3[4](uvec3(0u), uvec3(0u), uvec3(0u), uvec3(0u))));
  wbuffer = S(uvec3(0u), 0u, uvec3[4](uvec3(0u), uvec3(0u), uvec3(0u), uvec3(0u)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  foo_inner(gl_LocalInvocationIndex);
}
