builtins/gen/isInf/666f2a.wgsl:28:25 warning: use of deprecated builtin
  var res: vec3<bool> = isInf(vec3<f32>());
                        ^^^^^

#version 310 es

void isInf_666f2a() {
  bvec3 res = isinf(vec3(0.0f, 0.0f, 0.0f));
}

vec4 vertex_main() {
  isInf_666f2a();
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

void isInf_666f2a() {
  bvec3 res = isinf(vec3(0.0f, 0.0f, 0.0f));
}

void fragment_main() {
  isInf_666f2a();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void isInf_666f2a() {
  bvec3 res = isinf(vec3(0.0f, 0.0f, 0.0f));
}

void compute_main() {
  isInf_666f2a();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
