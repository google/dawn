static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[2];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b1, space0) {
  uint4 x_10[2];
};

float f1_f1_(inout float a) {
  const float x_100 = a;
  return ddx(x_100);
}

void main_1() {
  float4 v2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float a_1 = 0.0f;
  float x_40 = 0.0f;
  float param = 0.0f;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_42 = asint(x_8[scalar_offset / 4][scalar_offset % 4]);
  const int x_45 = asint(x_8[1].x);
  const int x_48 = asint(x_8[1].x);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_51 = asint(x_8[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  x_GLF_color = float4(float(x_42), float(x_45), float(x_48), float(x_51));
  const float x_55 = gl_FragCoord.x;
  const float x_57 = asfloat(x_10[1].x);
  if ((x_55 < x_57)) {
    const float x_62 = v2.x;
    if (!((x_62 < 1.0f))) {
      const float x_68 = asfloat(x_10[1].x);
      const float x_70 = asfloat(x_10[1].x);
      const uint scalar_offset_2 = ((16u * uint(0))) / 4;
      const float x_72 = asfloat(x_10[scalar_offset_2 / 4][scalar_offset_2 % 4]);
      if ((x_70 > x_72)) {
        const uint scalar_offset_3 = ((16u * uint(0))) / 4;
        const float x_78 = asfloat(x_10[scalar_offset_3 / 4][scalar_offset_3 % 4]);
        param = x_78;
        const float x_79 = f1_f1_(param);
        x_40 = x_79;
      } else {
        const uint scalar_offset_4 = ((16u * uint(0))) / 4;
        const float x_81 = asfloat(x_10[scalar_offset_4 / 4][scalar_offset_4 % 4]);
        x_40 = x_81;
      }
      a_1 = (x_68 / x_40);
      const uint scalar_offset_5 = ((16u * uint(0))) / 4;
      const float x_85 = asfloat(x_10[scalar_offset_5 / 4][scalar_offset_5 % 4]);
      const uint scalar_offset_6 = ((16u * uint(0))) / 4;
      const float x_88 = asfloat(x_10[scalar_offset_6 / 4][scalar_offset_6 % 4]);
      const float x_90 = a_1;
      const float3 x_92 = lerp(float3(x_85, x_85, x_85), float3(x_88, x_88, x_88), float3(x_90, x_90, x_90));
      const float x_94 = asfloat(x_10[1].x);
      x_GLF_color = float4(x_92.x, x_92.y, x_92.z, x_94);
    }
  }
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
  const main_out tint_symbol_5 = {x_GLF_color};
  return tint_symbol_5;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
