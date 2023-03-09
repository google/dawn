#version 310 es

uvec4 tint_select(uvec4 param_0, uvec4 param_1, bvec4 param_2) {
    return uvec4(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2], param_2[3] ? param_1[3] : param_0[3]);
}


layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
uvec4 tint_ftou(vec4 v) {
  return tint_select(uvec4(4294967295u), tint_select(uvec4(v), uvec4(0u), lessThan(v, vec4(0.0f))), lessThan(v, vec4(4294967040.0f)));
}

vec4 u = vec4(1.0f);
void f() {
  uvec4 v = tint_ftou(u);
}

