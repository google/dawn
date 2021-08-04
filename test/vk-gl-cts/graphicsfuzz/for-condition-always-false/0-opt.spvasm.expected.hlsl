static float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float3 drawShape_vf2_(inout float2 pos) {
  bool c3 = false;
  bool x_35_phi = false;
  const float x_32 = pos.y;
  const bool x_33 = (x_32 < 1.0f);
  c3 = x_33;
  x_35_phi = x_33;
  {
    for(; x_35_phi; x_35_phi = false) {
      return float3(1.0f, 1.0f, 1.0f);
    }
  }
  return float3(1.0f, 1.0f, 1.0f);
}

void main_1() {
  float2 param = float2(0.0f, 0.0f);
  param = float2(1.0f, 1.0f);
  const float3 x_29 = drawShape_vf2_(param);
  color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  return;
}

struct main_out {
  float4 color_1;
};
struct tint_symbol {
  float4 color_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_1 = {color};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.color_1 = inner_result.color_1;
  return wrapper_result;
}
