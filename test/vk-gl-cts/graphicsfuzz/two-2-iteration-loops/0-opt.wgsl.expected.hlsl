static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[1];
};

void main_1() {
  int odd_index = 0;
  int even_index = 0;
  int j = 0;
  int ll = 0;
  bool x_59 = false;
  bool x_60_phi = false;
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  const float x_53 = gl_FragCoord.x;
  const bool x_54 = (x_53 < 128.0f);
  x_60_phi = x_54;
  if (x_54) {
    const float x_58 = gl_FragCoord.y;
    x_59 = (x_58 < 128.0f);
    x_60_phi = x_59;
  }
  if (x_60_phi) {
    return;
  }
  odd_index = 0;
  while (true) {
    if ((odd_index <= 1)) {
    } else {
      break;
    }
    const float x_70 = x_GLF_color.x;
    x_GLF_color.x = (x_70 + 0.25f);
    odd_index = (odd_index + 1);
  }
  even_index = 1;
  while (true) {
    if ((even_index >= 0)) {
    } else {
      break;
    }
    const float x_80 = x_GLF_color.x;
    x_GLF_color.x = (x_80 + 0.25f);
    const float x_84 = asfloat(x_8[0].x);
    const float x_86 = asfloat(x_8[0].y);
    if ((x_84 > x_86)) {
      continue;
    }
    if ((even_index >= 1)) {
      discard;
    }
    j = 1;
    {
      for(; true; j = (j + 1)) {
        if ((ll >= 3)) {
          break;
        }
        ll = (ll + 1);
        if ((asuint(j) < 1u)) {
          continue;
        }
        const float x_106 = asfloat(x_8[0].x);
        const float x_108 = asfloat(x_8[0].y);
        if ((x_106 > x_108)) {
          break;
        }
      }
    }
    const float x_113 = asfloat(x_8[0].x);
    const float x_115 = asfloat(x_8[0].y);
    if ((x_113 > x_115)) {
      x_GLF_color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    even_index = (even_index - 1);
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
