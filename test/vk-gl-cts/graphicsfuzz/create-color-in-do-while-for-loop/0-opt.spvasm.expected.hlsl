void set_float2(inout float2 vec, int idx, float val) {
  vec = (idx.xx == int2(0, 1)) ? val.xx : vec;
}

cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float2 v = float2(0.0f, 0.0f);
  float floats[9] = (float[9])0;
  int one = 0;
  int i = 0;
  bool alwaysFalse = false;
  v = float2(0.0f, 0.0f);
  floats[1] = 0.0f;
  const float x_46 = asfloat(x_9[0].y);
  one = int(x_46);
  while (true) {
    i = 0;
    {
      for(; (i < one); i = (i + 1)) {
        if ((i == 0)) {
          const float x_65 = asfloat(x_9[0].x);
          const float x_67 = asfloat(x_9[0].y);
          alwaysFalse = (x_65 > x_67);
          if (!(alwaysFalse)) {
            floats[one] = 1.0f;
            x_GLF_color = float4(1.0f, 1.0f, 0.0f, 1.0f);
          }
          set_float2(v, one, 1.0f);
          if (alwaysFalse) {
            discard;
          }
          const float x_81 = asfloat(x_9[0].y);
          if ((x_81 < 0.0f)) {
            x_GLF_color = float4(0.0f, 1.0f, 0.0f, 1.0f);
          }
        }
      }
    }
    {
      if ((one < 0)) {
      } else {
        break;
      }
    }
  }
  bool x_102 = false;
  bool x_103_phi = false;
  const float x_90 = gl_FragCoord.y;
  if ((x_90 >= 0.0f)) {
    const float x_96 = v.y;
    const bool x_97 = (x_96 == 1.0f);
    x_103_phi = x_97;
    if (x_97) {
      const float x_101 = floats[1];
      x_102 = (x_101 == 1.0f);
      x_103_phi = x_102;
    }
    if (x_103_phi) {
      x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    }
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
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
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
