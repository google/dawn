static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[4];
};
cbuffer cbuffer_x_12 : register(b0, space0) {
  uint4 x_12[2];
};
static float4 x_GLF_v1 = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float2 uv = float2(0.0f, 0.0f);
  float4 v1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float a = 0.0f;
  int i = 0;
  const float4 x_49 = gl_FragCoord;
  uv = float2(x_49.x, x_49.y);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_52 = asfloat(x_8[scalar_offset / 4][scalar_offset % 4]);
  v1 = float4(x_52, x_52, x_52, x_52);
  const float x_55 = uv.y;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_57 = asfloat(x_8[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  if ((x_55 >= x_57)) {
    const float x_62 = asfloat(x_8[2].x);
    v1.x = x_62;
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const float x_65 = asfloat(x_8[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    v1.y = x_65;
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const float x_68 = asfloat(x_8[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    v1.z = x_68;
    const float x_71 = asfloat(x_8[2].x);
    v1.w = x_71;
  }
  const float x_74 = asfloat(x_8[2].x);
  a = x_74;
  const int x_15 = asint(x_12[1].x);
  i = x_15;
  while (true) {
    const int x_16 = i;
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_17 = asint(x_12[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    if ((x_16 < x_17)) {
    } else {
      break;
    }
    const float x_84 = asfloat(x_8[2].x);
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const float x_86 = asfloat(x_8[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    if ((x_84 < x_86)) {
      discard;
    }
    const float x_91 = v1.x;
    const float x_93 = v1.y;
    const float x_96 = v1.z;
    const float x_99 = v1.w;
    const float x_102 = asfloat(x_8[3].x);
    a = pow((((x_91 + x_93) + x_96) + x_99), x_102);
    {
      i = (i + 1);
    }
  }
  const float x_104 = a;
  const float x_106 = asfloat(x_8[1].x);
  if ((x_104 == x_106)) {
    x_GLF_v1 = v1;
  } else {
    const int x_20 = asint(x_12[1].x);
    const float x_113 = float(x_20);
    x_GLF_v1 = float4(x_113, x_113, x_113, x_113);
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
