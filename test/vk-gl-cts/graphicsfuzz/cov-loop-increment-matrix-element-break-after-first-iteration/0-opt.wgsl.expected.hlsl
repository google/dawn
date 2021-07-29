SKIP: FAILED

void set_float3(inout float3 vec, int idx, float val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

cbuffer cbuffer_x_7 : register(b1, space0) {
  uint4 x_7[2];
};
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[4];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float2x3 m23 = float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int i = 0;
  const float x_46 = asfloat(x_7[1].x);
  m23 = float2x3(float3(x_46, 0.0f, 0.0f), float3(0.0f, x_46, 0.0f));
  i = 1;
  while (true) {
    bool x_80 = false;
    bool x_81_phi = false;
    const int x_54 = i;
    const int x_56 = asint(x_10[3].x);
    if ((x_54 < x_56)) {
    } else {
      break;
    }
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_60 = asint(x_10[scalar_offset / 4][scalar_offset % 4]);
    const int x_62 = asint(x_10[2].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const float x_64 = asfloat(x_7[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const float x_66 = m23[x_60][x_62];
    set_float3(m23[x_60], x_62, (x_66 + x_64));
    const float x_70 = gl_FragCoord.y;
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const float x_72 = asfloat(x_7[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    if ((x_70 < x_72)) {
    }
    x_81_phi = true;
    if (true) {
      const float x_79 = gl_FragCoord.x;
      x_80 = (x_79 < 0.0f);
      x_81_phi = x_80;
    }
    if (!(x_81_phi)) {
      break;
    }
    {
      i = (i + 1);
    }
  }
  const float2x3 x_87 = m23;
  const int x_89 = asint(x_10[1].x);
  const int x_92 = asint(x_10[1].x);
  const int x_95 = asint(x_10[1].x);
  const int x_98 = asint(x_10[1].x);
  const int x_101 = asint(x_10[1].x);
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const int x_104 = asint(x_10[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  const float2x3 x_108 = float2x3(float3(float(x_89), float(x_92), float(x_95)), float3(float(x_98), float(x_101), float(x_104)));
  bool tint_tmp = all((x_87[0u] == x_108[0u]));
  if (tint_tmp) {
    tint_tmp = all((x_87[1u] == x_108[1u]));
  }
  if ((tint_tmp)) {
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_122 = asint(x_10[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    const int x_125 = asint(x_10[1].x);
    const int x_128 = asint(x_10[1].x);
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const int x_131 = asint(x_10[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    x_GLF_color = float4(float(x_122), float(x_125), float(x_128), float(x_131));
  } else {
    const int x_135 = asint(x_10[1].x);
    const float x_136 = float(x_135);
    x_GLF_color = float4(x_136, x_136, x_136, x_136);
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

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const float4 gl_FragCoord_param = tint_symbol.gl_FragCoord_param;
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_3 = {x_GLF_color};
  const tint_symbol_2 tint_symbol_6 = {tint_symbol_3.x_GLF_color_1};
  return tint_symbol_6;
}
C:\src\tint\test\Shader@0x000001FE6250D010(35,16-24): error X3500: array reference cannot be used as an l-value; not natively addressable
C:\src\tint\test\Shader@0x000001FE6250D010(20,3-14): error X3511: forced to unroll loop, but unrolling failed.

