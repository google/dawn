#version 310 es

mat2x3 m = mat2x3(vec3(0.0f), vec3(0.0f));
mat2x3 tint_symbol;
void tint_store_and_preserve_padding(inout mat2x3 target, mat2x3 value_param) {
  target[0u] = value_param[0u];
  target[1u] = value_param[1u];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_store_and_preserve_padding(tint_symbol, m);
}
