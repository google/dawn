cbuffer cbuffer_x_7 : register(b1, space0) {
  uint4 x_7[3];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_11 : register(b0, space0) {
  uint4 x_11[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int i = 0;
  int arr[2] = (int[2])0;
  int a = 0;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_40 = asint(x_7[scalar_offset / 4][scalar_offset % 4]);
  i = x_40;
  while (true) {
    const int x_45 = i;
    const int x_47 = asint(x_7[2].x);
    if ((x_45 < x_47)) {
    } else {
      break;
    }
    const int x_50 = i;
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_52 = asint(x_7[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    arr[x_50] = x_52;
    {
      i = (i + 1);
    }
  }
  a = -1;
  const float x_57 = gl_FragCoord.y;
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const float x_59 = asfloat(x_11[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  if (!((x_57 < x_59))) {
    const int x_65 = (a + 1);
    a = x_65;
    const int x_67 = asint(x_7[1].x);
    arr[x_65] = x_67;
  }
  const int x_70 = (a + 1);
  a = x_70;
  const int x_72 = asint(x_7[2].x);
  arr[x_70] = x_72;
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const int x_75 = asint(x_7[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  const int x_77 = arr[x_75];
  const int x_79 = asint(x_7[1].x);
  if ((x_77 == x_79)) {
    const int x_84 = a;
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_87 = asint(x_7[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const int x_90 = asint(x_7[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    x_GLF_color = float4(float(x_84), float(x_87), float(x_90), float(a));
  } else {
    const uint scalar_offset_6 = ((16u * uint(0))) / 4;
    const int x_96 = asint(x_7[scalar_offset_6 / 4][scalar_offset_6 % 4]);
    const float x_97 = float(x_96);
    x_GLF_color = float4(x_97, x_97, x_97, x_97);
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
