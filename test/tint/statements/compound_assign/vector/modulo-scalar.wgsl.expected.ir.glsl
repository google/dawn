#version 310 es


struct S {
  ivec4 a;
};

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  S tint_symbol;
} v_1;
ivec4 tint_mod_v4i32(ivec4 lhs, ivec4 rhs) {
  int v_2 = ((((rhs == ivec4(0)) | ((lhs == ivec4((-2147483647 - 1))) & (rhs == ivec4(-1)))).x) ? (ivec4(1).x) : (rhs.x));
  int v_3 = ((((rhs == ivec4(0)) | ((lhs == ivec4((-2147483647 - 1))) & (rhs == ivec4(-1)))).y) ? (ivec4(1).y) : (rhs.y));
  int v_4 = ((((rhs == ivec4(0)) | ((lhs == ivec4((-2147483647 - 1))) & (rhs == ivec4(-1)))).z) ? (ivec4(1).z) : (rhs.z));
  ivec4 v_5 = ivec4(v_2, v_3, v_4, ((((rhs == ivec4(0)) | ((lhs == ivec4((-2147483647 - 1))) & (rhs == ivec4(-1)))).w) ? (ivec4(1).w) : (rhs.w)));
  return (lhs - ((lhs / v_5) * v_5));
}
void foo() {
  ivec4 v_6 = v_1.tint_symbol.a;
  v_1.tint_symbol.a = tint_mod_v4i32(v_6, ivec4(2));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
