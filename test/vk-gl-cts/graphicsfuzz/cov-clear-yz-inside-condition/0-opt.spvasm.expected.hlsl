cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
cbuffer cbuffer_x_9 : register(b1, space0) {
  uint4 x_9[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float2 v0 = float2(0.0f, 0.0f);
  float4 v1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_57 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_32 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  v0 = float2(x_32, x_32);
  const float x_35 = v0.x;
  const float4 x_36 = float4(x_35, x_35, x_35, x_35);
  v1 = x_36;
  const float x_38 = asfloat(x_9[0].x);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_40 = asfloat(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  if (!((x_38 == x_40))) {
    const float x_46 = asfloat(x_9[0].x);
    const float x_48 = asfloat(x_6[1].x);
    if ((x_46 == x_48)) {
      return;
    }
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const float x_53 = asfloat(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const float2 x_56 = (float2(x_36.y, x_36.z) - float2(x_53, x_53));
    x_57 = float4(x_36.x, x_56.x, x_56.y, x_36.w);
    v1 = x_57;
  } else {
    discard;
  }
  x_GLF_color = x_57;
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
