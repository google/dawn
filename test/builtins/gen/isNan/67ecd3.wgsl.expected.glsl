builtins/gen/isNan/67ecd3.wgsl:28:25 warning: use of deprecated builtin
  var res: vec2<bool> = isNan(vec2<f32>());
                        ^^^^^

#version 310 es
precision mediump float;

void isNan_67ecd3() {
  bvec2 res = isnan(vec2(0.0f, 0.0f));
}

vec4 vertex_main() {
  isNan_67ecd3();
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

void isNan_67ecd3() {
  bvec2 res = isnan(vec2(0.0f, 0.0f));
}

void fragment_main() {
  isNan_67ecd3();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
precision mediump float;

void isNan_67ecd3() {
  bvec2 res = isnan(vec2(0.0f, 0.0f));
}

void compute_main() {
  isNan_67ecd3();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
