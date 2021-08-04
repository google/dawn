cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[3];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_12 : register(b0, space0) {
  uint4 x_12[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

int f1_() {
  int i = 0;
  int A[10] = (int[10])0;
  int a = 0;
  const int x_56 = asint(x_8[2].x);
  i = x_56;
  while (true) {
    const int x_61 = i;
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_63 = asint(x_8[scalar_offset / 4][scalar_offset % 4]);
    if ((x_61 < x_63)) {
    } else {
      break;
    }
    const int x_66 = i;
    const int x_68 = asint(x_8[2].x);
    A[x_66] = x_68;
    {
      i = (i + 1);
    }
  }
  a = -1;
  const float x_73 = gl_FragCoord.y;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_75 = asfloat(x_12[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  if ((x_73 >= x_75)) {
    const int x_80 = (a + 1);
    a = x_80;
    const int x_82 = asint(x_8[1].x);
    A[x_80] = x_82;
  }
  const int x_85 = asint(x_8[2].x);
  const int x_87 = A[x_85];
  const int x_89 = asint(x_8[1].x);
  if ((x_87 == x_89)) {
    const int x_95 = (a + 1);
    a = x_95;
    const int x_97 = A[x_95];
    return x_97;
  } else {
    const int x_99 = asint(x_8[1].x);
    return x_99;
  }
  return 0;
}

void main_1() {
  int i_1 = 0;
  const int x_42 = f1_();
  i_1 = x_42;
  const int x_44 = asint(x_8[1].x);
  const int x_46 = i_1;
  const int x_48 = i_1;
  const int x_51 = asint(x_8[1].x);
  x_GLF_color = float4(float(x_44), float(x_46), float(x_48), float(x_51));
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
