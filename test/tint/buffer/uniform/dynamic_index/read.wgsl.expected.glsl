#version 310 es

struct Inner {
  ivec3 a;
  int b;
  uvec3 c;
  uint d;
  vec3 e;
  float f;
  ivec2 g;
  ivec2 h;
  mat2x3 i;
  mat3x2 j;
  ivec4 k[4];
};

struct Inner_std140 {
  ivec3 a;
  int b;
  uvec3 c;
  uint d;
  vec3 e;
  float f;
  ivec2 g;
  ivec2 h;
  mat2x3 i;
  vec2 j_0;
  vec2 j_1;
  vec2 j_2;
  uint pad;
  uint pad_1;
  ivec4 k[4];
};

struct S {
  Inner arr[8];
};

layout(binding = 0, std140) uniform S_std140_ubo {
  Inner_std140 arr[8];
} s;

mat3x2 load_s_arr_p0_j(uint p0) {
  uint s_save = p0;
  return mat3x2(s.arr[s_save].j_0, s.arr[s_save].j_1, s.arr[s_save].j_2);
}

void tint_symbol(uint idx) {
  ivec3 a = s.arr[idx].a;
  int b = s.arr[idx].b;
  uvec3 c = s.arr[idx].c;
  uint d = s.arr[idx].d;
  vec3 e = s.arr[idx].e;
  float f = s.arr[idx].f;
  ivec2 g = s.arr[idx].g;
  ivec2 h = s.arr[idx].h;
  mat2x3 i = s.arr[idx].i;
  mat3x2 j = load_s_arr_p0_j(uint(idx));
  ivec4 k[4] = s.arr[idx].k;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
