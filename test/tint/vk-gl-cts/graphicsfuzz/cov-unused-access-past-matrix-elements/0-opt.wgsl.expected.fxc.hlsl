SKIP: FAILED

void set_scalar_float4x3(inout float4x3 mat, int col, int row, float val) {
  switch (col) {
    case 0:
      mat[0] = (row.xxx == int3(0, 1, 2)) ? val.xxx : mat[0];
      break;
    case 1:
      mat[1] = (row.xxx == int3(0, 1, 2)) ? val.xxx : mat[1];
      break;
    case 2:
      mat[2] = (row.xxx == int3(0, 1, 2)) ? val.xxx : mat[2];
      break;
    case 3:
      mat[3] = (row.xxx == int3(0, 1, 2)) ? val.xxx : mat[3];
      break;
  }
}

struct tint_padded_array_element {
  float el;
};

cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[3];
};
cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4x3 m43 = float4x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  tint_padded_array_element sums[3] = (tint_padded_array_element[3])0;
  int i = 0;
  int a = 0;
  int x_67_phi = 0;
  const float x_44 = asfloat(x_6[1].x);
  const float3 x_48 = float3(0.0f, 0.0f, 0.0f);
  m43 = float4x3(float3(x_44, 0.0f, 0.0f), float3(0.0f, x_44, 0.0f), float3(0.0f, 0.0f, x_44), float3(0.0f, 0.0f, 0.0f));
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_51 = asint(x_8[scalar_offset / 4][scalar_offset % 4]);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_53 = asint(x_8[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const float x_55 = asfloat(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  set_scalar_float4x3(m43, x_53, x_51, x_55);
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const float x_58 = asfloat(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  const uint scalar_offset_4 = ((16u * uint(0))) / 4;
  const float x_60 = asfloat(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
  const uint scalar_offset_5 = ((16u * uint(0))) / 4;
  const float x_62 = asfloat(x_6[scalar_offset_5 / 4][scalar_offset_5 % 4]);
  const tint_padded_array_element tint_symbol_3[3] = {{x_58}, {x_60}, {x_62}};
  sums = tint_symbol_3;
  const uint scalar_offset_6 = ((16u * uint(0))) / 4;
  const int x_65 = asint(x_8[scalar_offset_6 / 4][scalar_offset_6 % 4]);
  i = x_65;
  x_67_phi = x_65;
  [loop] while (true) {
    const int x_67 = x_67_phi;
    const int x_73 = asint(x_8[3].x);
    if ((x_67 < x_73)) {
    } else {
      break;
    }
    const uint scalar_offset_7 = ((16u * uint(0))) / 4;
    const int x_77 = asint(x_8[scalar_offset_7 / 4][scalar_offset_7 % 4]);
    const uint scalar_offset_8 = ((16u * uint(0))) / 4;
    const int x_79 = asint(x_8[scalar_offset_8 / 4][scalar_offset_8 % 4]);
    const float x_81 = m43[x_67][x_79];
    const float x_83 = sums[x_77].el;
    sums[x_77].el = (x_83 + x_81);
    {
      const int x_68 = (x_67 + 1);
      i = x_68;
      x_67_phi = x_68;
    }
  }
  const int x_87 = asint(x_8[1].x);
  if ((x_87 == 1)) {
    a = 4;
    const int x_92 = asint(x_8[2].x);
    const uint scalar_offset_9 = ((16u * uint(0))) / 4;
    const int x_94 = asint(x_8[scalar_offset_9 / 4][scalar_offset_9 % 4]);
    const float x_96 = m43[4][x_94];
    const float x_98 = sums[x_92].el;
    sums[x_92].el = (x_98 + x_96);
  }
  const int x_102 = asint(x_8[1].x);
  const float x_104 = sums[x_102].el;
  const uint scalar_offset_10 = ((16u * uint(0))) / 4;
  const int x_106 = asint(x_8[scalar_offset_10 / 4][scalar_offset_10 % 4]);
  const float x_108 = sums[x_106].el;
  const float x_111 = asfloat(x_6[2].x);
  if (((x_104 + x_108) == x_111)) {
    const uint scalar_offset_11 = ((16u * uint(0))) / 4;
    const int x_117 = asint(x_8[scalar_offset_11 / 4][scalar_offset_11 % 4]);
    const int x_120 = asint(x_8[1].x);
    const int x_123 = asint(x_8[1].x);
    const uint scalar_offset_12 = ((16u * uint(0))) / 4;
    const int x_126 = asint(x_8[scalar_offset_12 / 4][scalar_offset_12 % 4]);
    x_GLF_color = float4(float(x_117), float(x_120), float(x_123), float(x_126));
  } else {
    const int x_130 = asint(x_8[1].x);
    const float x_131 = float(x_130);
    x_GLF_color = float4(x_131, x_131, x_131, x_131);
  }
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
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
C:\src\tint\test\Shader@0x0000020DD7F92800(84,24-29): error X3504: array index out of bounds

