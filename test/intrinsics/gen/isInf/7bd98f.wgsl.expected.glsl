intrinsics/gen/isInf/7bd98f.wgsl:28:19 warning: use of deprecated intrinsic
  var res: bool = isInf(1.0);
                  ^^^^^

#version 310 es
precision mediump float;

void isInf_7bd98f() {
  bool res = isinf(1.0f);
}

vec4 vertex_main() {
  isInf_7bd98f();
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

void isInf_7bd98f() {
  bool res = isinf(1.0f);
}

void fragment_main() {
  isInf_7bd98f();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
precision mediump float;

void isInf_7bd98f() {
  bool res = isinf(1.0f);
}

void compute_main() {
  isInf_7bd98f();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
