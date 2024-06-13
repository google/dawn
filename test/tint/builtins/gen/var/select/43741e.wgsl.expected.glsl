#version 310 es
precision highp float;
precision highp int;

vec4 tint_select(vec4 param_0, vec4 param_1, bvec4 param_2) {
    return vec4(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2], param_2[3] ? param_1[3] : param_0[3]);
}


void select_43741e() {
  bvec4 arg_2 = bvec4(true);
  vec4 res = tint_select(vec4(1.0f), vec4(1.0f), arg_2);
}

struct VertexOutput {
  vec4 pos;
};

void fragment_main() {
  select_43741e();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

vec4 tint_select(vec4 param_0, vec4 param_1, bvec4 param_2) {
    return vec4(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2], param_2[3] ? param_1[3] : param_0[3]);
}


void select_43741e() {
  bvec4 arg_2 = bvec4(true);
  vec4 res = tint_select(vec4(1.0f), vec4(1.0f), arg_2);
}

struct VertexOutput {
  vec4 pos;
};

void compute_main() {
  select_43741e();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es

vec4 tint_select(vec4 param_0, vec4 param_1, bvec4 param_2) {
    return vec4(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2], param_2[3] ? param_1[3] : param_0[3]);
}


void select_43741e() {
  bvec4 arg_2 = bvec4(true);
  vec4 res = tint_select(vec4(1.0f), vec4(1.0f), arg_2);
}

struct VertexOutput {
  vec4 pos;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  tint_symbol.pos = vec4(0.0f);
  select_43741e();
  return tint_symbol;
}

void main() {
  gl_PointSize = 1.0;
  VertexOutput inner_result = vertex_main();
  gl_Position = inner_result.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
