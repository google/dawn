#version 310 es

void smoothstep_6c4975() {
  float arg_0 = 2.0f;
  float arg_1 = 4.0f;
  float arg_2 = 3.0f;
  float res = smoothstep(arg_0, arg_1, arg_2);
}

vec4 vertex_main() {
  smoothstep_6c4975();
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

void smoothstep_6c4975() {
  float arg_0 = 2.0f;
  float arg_1 = 4.0f;
  float arg_2 = 3.0f;
  float res = smoothstep(arg_0, arg_1, arg_2);
}

void fragment_main() {
  smoothstep_6c4975();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void smoothstep_6c4975() {
  float arg_0 = 2.0f;
  float arg_1 = 4.0f;
  float arg_2 = 3.0f;
  float res = smoothstep(arg_0, arg_1, arg_2);
}

void compute_main() {
  smoothstep_6c4975();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
