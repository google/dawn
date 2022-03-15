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
  vec2 v2 = vec2(tint_symbol);
  float tint_symbol_1 = get_f32();
  vec3 v3 = vec3(tint_symbol_1);
  float tint_symbol_2 = get_f32();
  vec4 v4 = vec4(tint_symbol_2);
}

