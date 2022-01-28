intrinsics/gen/ignore/d91a2f.wgsl:28:3 warning: use of deprecated intrinsic
  ignore(1.0);
  ^^^^^^

#version 310 es
precision mediump float;

void ignore_d91a2f() {
  1.0f;
}

vec4 vertex_main() {
  ignore_d91a2f();
  return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

void main() {
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision mediump float;

void ignore_d91a2f() {
  1.0f;
}

void fragment_main() {
  ignore_d91a2f();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
precision mediump float;

void ignore_d91a2f() {
  1.0f;
}

void compute_main() {
  ignore_d91a2f();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
