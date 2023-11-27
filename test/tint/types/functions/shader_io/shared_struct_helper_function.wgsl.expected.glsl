#version 310 es

layout(location = 0) flat out int loc0_1;
struct VertexOutput {
  vec4 pos;
  int loc0;
};

VertexOutput foo(float x) {
  VertexOutput tint_symbol = VertexOutput(vec4(x, x, x, 1.0f), 42);
  return tint_symbol;
}

VertexOutput vert_main1() {
  return foo(0.5f);
}

void main() {
  gl_PointSize = 1.0;
  VertexOutput inner_result = vert_main1();
  gl_Position = inner_result.pos;
  loc0_1 = inner_result.loc0;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es

layout(location = 0) flat out int loc0_1;
struct VertexOutput {
  vec4 pos;
  int loc0;
};

VertexOutput foo(float x) {
  VertexOutput tint_symbol = VertexOutput(vec4(x, x, x, 1.0f), 42);
  return tint_symbol;
}

VertexOutput vert_main2() {
  return foo(0.25f);
}

void main() {
  gl_PointSize = 1.0;
  VertexOutput inner_result = vert_main2();
  gl_Position = inner_result.pos;
  loc0_1 = inner_result.loc0;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
