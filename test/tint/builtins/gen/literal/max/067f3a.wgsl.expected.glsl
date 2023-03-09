#version 310 es

void max_067f3a() {
  ivec2 res = ivec2(1);
}

vec4 vertex_main() {
  max_067f3a();
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

void max_067f3a() {
  ivec2 res = ivec2(1);
}

void fragment_main() {
  max_067f3a();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void max_067f3a() {
  ivec2 res = ivec2(1);
}

void compute_main() {
  max_067f3a();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
