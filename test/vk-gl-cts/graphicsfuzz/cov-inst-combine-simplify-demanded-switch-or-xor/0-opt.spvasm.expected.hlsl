void set_float4(inout float4 vec, int idx, float val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_29 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_31 = asfloat(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const float x_33 = asfloat(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  const float x_35 = asfloat(x_6[1].x);
  color = float4(x_29, x_31, x_33, x_35);
  const int x_38 = asint(x_8[1].x);
  switch(((1 | x_38) ^ 1)) {
    case 0: {
      const uint scalar_offset_3 = ((16u * uint(0))) / 4;
      const int x_44 = asint(x_8[scalar_offset_3 / 4][scalar_offset_3 % 4]);
      const float x_46 = asfloat(x_6[1].x);
      set_float4(color, x_44, x_46);
      break;
    }
    default: {
      break;
    }
  }
  x_GLF_color = color;
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
