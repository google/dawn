#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct S {
  uvec3 a;
  uint b;
  uvec3 c[4];
};

layout(binding = 0, std140) uniform ubuffer_block_ubo {
  S inner;
} ubuffer;

layout(binding = 1, std430) buffer ubuffer_block_ssbo {
  S inner;
} sbuffer;

shared S wbuffer;
void assign_and_preserve_padding_1_sbuffer_inner_c(uvec3 value[4]) {
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      sbuffer.inner.c[i] = value[i];
    }
  }
}

void assign_and_preserve_padding_sbuffer_inner(S value) {
  sbuffer.inner.a = value.a;
  sbuffer.inner.b = value.b;
  assign_and_preserve_padding_1_sbuffer_inner_c(value.c);
}

void foo() {
  S u = ubuffer.inner;
  S s = sbuffer.inner;
  S w = sbuffer.inner;
  S tint_symbol = S(uvec3(0u), 0u, uvec3[4](uvec3(0u), uvec3(0u), uvec3(0u), uvec3(0u)));
  assign_and_preserve_padding_sbuffer_inner(tint_symbol);
  S tint_symbol_1 = S(uvec3(0u), 0u, uvec3[4](uvec3(0u), uvec3(0u), uvec3(0u), uvec3(0u)));
  wbuffer = tint_symbol_1;
}

