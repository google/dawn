static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[3];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_9 : register(b1, space0) {
  uint4 x_9[1];
};

void main_1() {
  int i = 0;
  const int x_34 = asint(x_6[2].x);
  const float x_35 = float(x_34);
  x_GLF_color = float4(x_35, x_35, x_35, x_35);
  const float x_38 = gl_FragCoord.y;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_40 = asfloat(x_9[scalar_offset / 4][scalar_offset % 4]);
  i = (1 << asuint(((x_38 >= x_40) ? 2 : 1)));
  while (true) {
    bool x_57 = false;
    bool x_58_phi = false;
    const int x_48 = i;
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_50 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const bool x_51 = (x_48 != x_50);
    x_58_phi = x_51;
    if (x_51) {
      const int x_54 = i;
      const int x_56 = asint(x_6[1].x);
      x_57 = (x_54 < x_56);
      x_58_phi = x_57;
    }
    if (x_58_phi) {
    } else {
      break;
    }
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_61 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const int x_64 = asint(x_6[2].x);
    const int x_67 = asint(x_6[2].x);
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_70 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    x_GLF_color = float4(float(x_61), float(x_64), float(x_67), float(x_70));
    {
      i = (i + 1);
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
