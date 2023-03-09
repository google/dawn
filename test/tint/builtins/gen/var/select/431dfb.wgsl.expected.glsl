#version 310 es

ivec2 tint_select(ivec2 param_0, ivec2 param_1, bvec2 param_2) {
    return ivec2(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1]);
}


void select_431dfb() {
  bvec2 arg_2 = bvec2(true);
  ivec2 res = tint_select(ivec2(1), ivec2(1), arg_2);
}

vec4 vertex_main() {
  select_431dfb();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision highp float;

ivec2 tint_select(ivec2 param_0, ivec2 param_1, bvec2 param_2) {
    return ivec2(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1]);
}


void select_431dfb() {
  bvec2 arg_2 = bvec2(true);
  ivec2 res = tint_select(ivec2(1), ivec2(1), arg_2);
}

void fragment_main() {
  select_431dfb();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

ivec2 tint_select(ivec2 param_0, ivec2 param_1, bvec2 param_2) {
    return ivec2(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1]);
}


void select_431dfb() {
  bvec2 arg_2 = bvec2(true);
  ivec2 res = tint_select(ivec2(1), ivec2(1), arg_2);
}

void compute_main() {
  select_431dfb();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
