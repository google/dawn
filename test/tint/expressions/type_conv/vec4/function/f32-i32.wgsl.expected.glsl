#version 310 es

ivec4 tint_select(ivec4 param_0, ivec4 param_1, bvec4 param_2) {
    return ivec4(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2], param_2[3] ? param_1[3] : param_0[3]);
}


layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
ivec4 tint_ftoi(vec4 v) {
  return tint_select(ivec4(2147483647), tint_select(ivec4(v), ivec4((-2147483647 - 1)), lessThan(v, vec4(-2147483648.0f))), lessThan(v, vec4(2147483520.0f)));
}

float t = 0.0f;
vec4 m() {
  t = 1.0f;
  return vec4(t);
}

void f() {
  vec4 tint_symbol = m();
  ivec4 v = tint_ftoi(tint_symbol);
}

