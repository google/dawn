builtins/gen/smoothstep/658be3.wgsl:28:24 warning: use of deprecated builtin
  var res: vec3<f32> = smoothStep(vec3<f32>(), vec3<f32>(), vec3<f32>());
                       ^^^^^^^^^^

#version 310 es

void smoothStep_658be3() {
  vec3 res = smoothstep(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));
}

vec4 vertex_main() {
  smoothStep_658be3();
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

void smoothStep_658be3() {
  vec3 res = smoothstep(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));
}

void fragment_main() {
  smoothStep_658be3();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void smoothStep_658be3() {
  vec3 res = smoothstep(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));
}

void compute_main() {
  smoothStep_658be3();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
