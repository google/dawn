#version 310 es

void select_28a27e() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 arg_1 = uvec3(1u);
  bvec3 arg_2 = bvec3(true);
  uvec3 res = mix(arg_0, arg_1, arg_2);
}

vec4 vertex_main() {
  select_28a27e();
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
precision mediump float;

void select_28a27e() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 arg_1 = uvec3(1u);
  bvec3 arg_2 = bvec3(true);
  uvec3 res = mix(arg_0, arg_1, arg_2);
}

void fragment_main() {
  select_28a27e();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void select_28a27e() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 arg_1 = uvec3(1u);
  bvec3 arg_2 = bvec3(true);
  uvec3 res = mix(arg_0, arg_1, arg_2);
}

void compute_main() {
  select_28a27e();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
