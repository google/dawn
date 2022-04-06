struct VertexOutput {
  float4 pos;
  int loc0;
};

VertexOutput foo(float x) {
  const VertexOutput tint_symbol_2 = {float4(x, x, x, 1.0f), 42};
  return tint_symbol_2;
}

struct tint_symbol {
  nointerpolation int loc0 : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vert_main1_inner() {
  return foo(0.5f);
}

tint_symbol vert_main1() {
  const VertexOutput inner_result = vert_main1_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.loc0 = inner_result.loc0;
  return wrapper_result;
}

struct tint_symbol_1 {
  nointerpolation int loc0 : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vert_main2_inner() {
  return foo(0.25f);
}

tint_symbol_1 vert_main2() {
  const VertexOutput inner_result_1 = vert_main2_inner();
  tint_symbol_1 wrapper_result_1 = (tint_symbol_1)0;
  wrapper_result_1.pos = inner_result_1.pos;
  wrapper_result_1.loc0 = inner_result_1.loc0;
  return wrapper_result_1;
}
