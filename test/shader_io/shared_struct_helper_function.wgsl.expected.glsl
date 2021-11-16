#version 310 es
precision mediump float;

struct VertexOutput {
  vec4 pos;
  int loc0;
};

VertexOutput foo(float x) {
  VertexOutput tint_symbol_2 = VertexOutput(vec4(x, x, x, 1.0f), 42);
  return tint_symbol_2;
}

struct tint_symbol {
  int loc0;
  vec4 pos;
};

VertexOutput vert_main1_inner() {
  return foo(0.5f);
}

struct tint_symbol_1 {
  int loc0;
  vec4 pos;
};

tint_symbol vert_main1() {
  VertexOutput inner_result = vert_main1_inner();
  tint_symbol wrapper_result = tint_symbol(0, vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.pos = inner_result.pos;
  wrapper_result.loc0 = inner_result.loc0;
  return wrapper_result;
}
out int loc0;
void main() {
  tint_symbol outputs;
  outputs = vert_main1();
  loc0 = outputs.loc0;
  gl_Position = outputs.pos;
  gl_Position.y = -gl_Position.y;
}


#version 310 es
precision mediump float;

struct VertexOutput {
  vec4 pos;
  int loc0;
};

VertexOutput foo(float x) {
  VertexOutput tint_symbol_2 = VertexOutput(vec4(x, x, x, 1.0f), 42);
  return tint_symbol_2;
}

struct tint_symbol {
  int loc0;
  vec4 pos;
};
struct tint_symbol_1 {
  int loc0;
  vec4 pos;
};

VertexOutput vert_main2_inner() {
  return foo(0.25f);
}

tint_symbol_1 vert_main2() {
  VertexOutput inner_result_1 = vert_main2_inner();
  tint_symbol_1 wrapper_result_1 = tint_symbol_1(0, vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result_1.pos = inner_result_1.pos;
  wrapper_result_1.loc0 = inner_result_1.loc0;
  return wrapper_result_1;
}
out int loc0;
void main() {
  tint_symbol_1 outputs;
  outputs = vert_main2();
  loc0 = outputs.loc0;
  gl_Position = outputs.pos;
  gl_Position.y = -gl_Position.y;
}


