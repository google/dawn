void set_float2(inout float2 vec, int idx, float val) {
  vec = (idx.xx == int2(0, 1)) ? val.xx : vec;
}

cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float2 v1 = float2(0.0f, 0.0f);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_35 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  v1 = float2(x_35, x_35);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_38 = asint(x_8[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  const float x_40 = v1.y;
  set_float2(v1, x_38, ldexp(x_40, -256));
  if ((mul(float2x2(float2(x_35, 0.0f), float2(0.0f, x_35)), v1).x == x_35)) {
    const float x_53 = float(x_38);
    const int x_55 = asint(x_8[1].x);
    const float x_56 = float(x_55);
    x_GLF_color = float4(x_53, x_56, x_56, x_53);
  } else {
    const int x_59 = asint(x_8[1].x);
    const float x_60 = float(x_59);
    x_GLF_color = float4(x_60, x_60, x_60, x_60);
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_3 = {x_GLF_color};
  return tint_symbol_3;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
