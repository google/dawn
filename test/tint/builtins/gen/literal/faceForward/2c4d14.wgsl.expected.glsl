#version 310 es
precision highp float;
precision highp int;

void faceForward_2c4d14() {
  vec4 res = vec4(-1.0f);
}

struct VertexOutput {
  vec4 pos;
};

void fragment_main() {
  faceForward_2c4d14();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void faceForward_2c4d14() {
  vec4 res = vec4(-1.0f);
}

struct VertexOutput {
  vec4 pos;
};

void compute_main() {
  faceForward_2c4d14();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es

void faceForward_2c4d14() {
  vec4 res = vec4(-1.0f);
}

struct VertexOutput {
  vec4 pos;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  tint_symbol.pos = vec4(0.0f);
  faceForward_2c4d14();
  return tint_symbol;
}

void main() {
  gl_PointSize = 1.0;
  VertexOutput inner_result = vertex_main();
  gl_Position = inner_result.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
