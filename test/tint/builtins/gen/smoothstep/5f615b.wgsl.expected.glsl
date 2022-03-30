builtins/gen/smoothstep/5f615b.wgsl:28:24 warning: use of deprecated builtin
  var res: vec4<f32> = smoothStep(vec4<f32>(), vec4<f32>(), vec4<f32>());
                       ^^^^^^^^^^

#version 310 es

void smoothStep_5f615b() {
  vec4 res = smoothstep(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

vec4 vertex_main() {
  smoothStep_5f615b();
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

void smoothStep_5f615b() {
  vec4 res = smoothstep(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

void fragment_main() {
  smoothStep_5f615b();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void smoothStep_5f615b() {
  vec4 res = smoothstep(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

void compute_main() {
  smoothStep_5f615b();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
