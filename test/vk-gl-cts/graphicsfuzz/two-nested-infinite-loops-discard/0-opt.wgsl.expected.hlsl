SKIP: FAILED

cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float3 mand_() {
  int k = 0;
  k = 0;
  while (true) {
    if (true) {
    } else {
      break;
    }
    discard;
  }
  return float3(1.0f, 1.0f, 1.0f);
}

void main_1() {
  int j = 0;
  const float x_37 = asfloat(x_7[0].x);
  const float x_39 = asfloat(x_7[0].y);
  if ((x_37 > x_39)) {
    j = 0;
    {
      for(; true; const float3 x_46 = mand_()) {
      }
    }
  }
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

tint_symbol main() {
  main_1();
  const main_out tint_symbol_1 = {x_GLF_color};
  const tint_symbol tint_symbol_3 = {tint_symbol_1.x_GLF_color_1};
  return tint_symbol_3;
}
T:\tmp\ub0s.0:26:19: error: expected expression
      for(; true; const float3 x_46 = mand_()) {
                  ^
T:\tmp\ub0s.0:26:19: error: expected ')'
T:\tmp\ub0s.0:26:10: note: to match this '('
      for(; true; const float3 x_46 = mand_()) {
         ^


