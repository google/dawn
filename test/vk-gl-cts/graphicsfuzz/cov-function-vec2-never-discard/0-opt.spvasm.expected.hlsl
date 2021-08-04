cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[2];
};
cbuffer cbuffer_x_10 : register(b2, space0) {
  uint4 x_10[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_13 : register(b0, space0) {
  uint4 x_13[2];
};

bool func_vf2_(inout float2 pos) {
  const float x_62 = pos.x;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_64 = asfloat(x_8[scalar_offset / 4][scalar_offset % 4]);
  if ((x_62 < x_64)) {
    return true;
  }
  const float x_69 = pos.y;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_71 = asfloat(x_8[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  if ((x_69 > x_71)) {
    return false;
  }
  const float x_76 = asfloat(x_10[0].x);
  const float x_78 = asfloat(x_8[1].x);
  if ((x_76 > x_78)) {
    return true;
  }
  return true;
}

void main_1() {
  float2 param = float2(0.0f, 0.0f);
  const float4 x_42 = gl_FragCoord;
  param = float2(x_42.x, x_42.y);
  const bool x_44 = func_vf2_(param);
  if (x_44) {
    discard;
  }
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const int x_48 = asint(x_13[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  const int x_51 = asint(x_13[1].x);
  const int x_54 = asint(x_13[1].x);
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const int x_57 = asint(x_13[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  x_GLF_color = float4(float(x_48), float(x_51), float(x_54), float(x_57));
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol_1 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_2 {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_6 = {x_GLF_color};
  return tint_symbol_6;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
