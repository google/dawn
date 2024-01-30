struct Interface {
  int i;
  uint u;
  int4 vi;
  uint4 vu;
  float4 pos;
};
struct tint_symbol {
  nointerpolation int i : TEXCOORD0;
  nointerpolation uint u : TEXCOORD1;
  nointerpolation int4 vi : TEXCOORD2;
  nointerpolation uint4 vu : TEXCOORD3;
  float4 pos : SV_Position;
};

Interface vert_main_inner() {
  Interface tint_symbol_4 = (Interface)0;
  return tint_symbol_4;
}

tint_symbol vert_main() {
  Interface inner_result = vert_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.i = inner_result.i;
  wrapper_result.u = inner_result.u;
  wrapper_result.vi = inner_result.vi;
  wrapper_result.vu = inner_result.vu;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}

struct tint_symbol_2 {
  nointerpolation int i : TEXCOORD0;
  nointerpolation uint u : TEXCOORD1;
  nointerpolation int4 vi : TEXCOORD2;
  nointerpolation uint4 vu : TEXCOORD3;
  float4 pos : SV_Position;
};
struct tint_symbol_3 {
  int value : SV_Target0;
};

int frag_main_inner(Interface inputs) {
  return inputs.i;
}

tint_symbol_3 frag_main(tint_symbol_2 tint_symbol_1) {
  Interface tint_symbol_5 = {tint_symbol_1.i, tint_symbol_1.u, tint_symbol_1.vi, tint_symbol_1.vu, float4(tint_symbol_1.pos.xyz, (1.0f / tint_symbol_1.pos.w))};
  int inner_result_1 = frag_main_inner(tint_symbol_5);
  tint_symbol_3 wrapper_result_1 = (tint_symbol_3)0;
  wrapper_result_1.value = inner_result_1;
  return wrapper_result_1;
}
