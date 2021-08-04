cbuffer cbuffer_x_7 : register(b1, space0) {
  uint4 x_7[3];
};
cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[2];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_v1 = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int i = 0;
  int j = 0;
  const int x_36 = asint(x_7[1].x);
  i = x_36;
  while (true) {
    const int x_41 = i;
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_43 = asint(x_7[scalar_offset / 4][scalar_offset % 4]);
    if ((x_41 < x_43)) {
    } else {
      break;
    }
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const float x_47 = asfloat(x_9[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const float x_49 = asfloat(x_9[1].x);
    if ((x_47 > x_49)) {
      discard;
    }
    const int x_54 = asint(x_7[1].x);
    j = x_54;
    while (true) {
      const int x_59 = j;
      const uint scalar_offset_2 = ((16u * uint(0))) / 4;
      const int x_61 = asint(x_7[scalar_offset_2 / 4][scalar_offset_2 % 4]);
      if ((x_59 < x_61)) {
      } else {
        break;
      }
      const float x_65 = gl_FragCoord.x;
      const uint scalar_offset_3 = ((16u * uint(0))) / 4;
      const float x_67 = asfloat(x_9[scalar_offset_3 / 4][scalar_offset_3 % 4]);
      if ((x_65 < x_67)) {
        discard;
      }
      const int x_72 = asint(x_7[2].x);
      const int x_75 = asint(x_7[1].x);
      const int x_78 = asint(x_7[1].x);
      const int x_81 = asint(x_7[2].x);
      x_GLF_v1 = float4(float(x_72), float(x_75), float(x_78), float(x_81));
      {
        j = (j + 1);
      }
    }
    {
      i = (i + 1);
    }
  }
  return;
}

struct main_out {
  float4 x_GLF_v1_1;
};
struct tint_symbol_1 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_2 {
  float4 x_GLF_v1_1 : SV_Target0;
};

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_5 = {x_GLF_v1};
  return tint_symbol_5;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_v1_1 = inner_result.x_GLF_v1_1;
  return wrapper_result;
}
