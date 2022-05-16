#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
float get_f32() {
  return 1.0f;
}

void f() {
  float tint_symbol = get_f32();
  mat2 m2x2 = mat2(tint_symbol);
  float tint_symbol_1 = get_f32();
  mat2x3 m2x3 = mat2x3(tint_symbol_1);
  float tint_symbol_2 = get_f32();
  mat2x4 m2x4 = mat2x4(tint_symbol_2);
  float tint_symbol_3 = get_f32();
  mat3x2 m3x2 = mat3x2(tint_symbol_3);
  float tint_symbol_4 = get_f32();
  mat3 m3x3 = mat3(tint_symbol_4);
  float tint_symbol_5 = get_f32();
  mat3x4 m3x4 = mat3x4(tint_symbol_5);
  float tint_symbol_6 = get_f32();
  mat4x2 m4x2 = mat4x2(tint_symbol_6);
  float tint_symbol_7 = get_f32();
  mat4x3 m4x3 = mat4x3(tint_symbol_7);
  float tint_symbol_8 = get_f32();
  mat4 m4x4 = mat4(tint_symbol_8);
}

