uint tint_pack4x8unorm(float4 param_0) {
  uint4 i = uint4(round(clamp(param_0, 0.0, 1.0) * 255.0));
  return (i.x | i.y << 8 | i.z << 16 | i.w << 24);
}

static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[1];
};

float func_() {
  switch(1) {
    case 0: {
      return 1.0f;
      break;
    }
    default: {
      break;
    }
  }
  return 0.0f;
}

void main_1() {
  float4 v = float4(0.0f, 0.0f, 0.0f, 0.0f);
  v = float4(1.0f, 1.0f, 1.0f, 1.0f);
  const float x_38 = gl_FragCoord.y;
  if ((x_38 < 0.0f)) {
    const float x_42 = func_();
    v = float4(x_42, x_42, x_42, x_42);
  }
  if ((tint_pack4x8unorm(v) == 1u)) {
    return;
  }
  const uint x_50 = x_8[0].x;
  if (((1u << x_50) == 2u)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_57 = asint(x_10[scalar_offset / 4][scalar_offset % 4]);
    const float x_58 = float(x_57);
    x_GLF_color = float4(x_58, x_58, x_58, x_58);
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
