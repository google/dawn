builtins/gen/smoothstep/cb0bfb.wgsl:28:18 warning: use of deprecated builtin
  var res: f32 = smoothStep(1.0, 1.0, 1.0);
                 ^^^^^^^^^^

#version 310 es

void smoothStep_cb0bfb() {
  float res = smoothstep(1.0f, 1.0f, 1.0f);
}

vec4 vertex_main() {
  smoothStep_cb0bfb();
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

void smoothStep_cb0bfb() {
  float res = smoothstep(1.0f, 1.0f, 1.0f);
}

void fragment_main() {
  smoothStep_cb0bfb();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void smoothStep_cb0bfb() {
  float res = smoothstep(1.0f, 1.0f, 1.0f);
}

void compute_main() {
  smoothStep_cb0bfb();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
