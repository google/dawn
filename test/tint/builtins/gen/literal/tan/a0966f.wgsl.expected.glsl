#version 310 es

void tan_a0966f() {
  vec4 res = vec4(1.55740773677825927734f);
}

vec4 vertex_main() {
  tan_a0966f();
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

void tan_a0966f() {
  vec4 res = vec4(1.55740773677825927734f);
}

void fragment_main() {
  tan_a0966f();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void tan_a0966f() {
  vec4 res = vec4(1.55740773677825927734f);
}

void compute_main() {
  tan_a0966f();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
