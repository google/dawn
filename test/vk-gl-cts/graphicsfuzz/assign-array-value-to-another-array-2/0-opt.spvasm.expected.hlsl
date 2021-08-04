static int data[9] = (int[9])0;
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static int temp[7] = (int[7])0;
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float func_i1_(inout int a) {
  int b = 0;
  int i = 0;
  bool x_115 = false;
  bool x_116_phi = false;
  b = 0;
  data[0] = 5;
  data[2] = 0;
  data[4] = 0;
  data[6] = 0;
  data[8] = 0;
  const float x_71 = gl_FragCoord.x;
  if ((x_71 >= 0.0f)) {
    while (true) {
      const int x_79 = b;
      const int x_80 = a;
      if ((x_79 <= x_80)) {
      } else {
        break;
      }
      if ((b <= 5)) {
        const int x_87 = b;
        const int x_90 = data[b];
        temp[x_87] = x_90;
        b = (b + 2);
      }
    }
  }
  i = 0;
  {
    for(; (i < 3); i = (i + 1)) {
      const int x_101 = i;
      const int x_103 = temp[0];
      data[x_101] = (x_103 + 1);
    }
  }
  const int x_109 = temp[0];
  const bool x_110 = (x_109 == 5);
  x_116_phi = x_110;
  if (x_110) {
    const int x_114 = data[0];
    x_115 = (x_114 == 6);
    x_116_phi = x_115;
  }
  if (x_116_phi) {
    return 1.0f;
  } else {
    return 0.0f;
  }
  return 0.0f;
}

void main_1() {
  int i_1 = 0;
  int param = 0;
  int param_1 = 0;
  i_1 = 0;
  {
    for(; (i_1 < 6); i_1 = (i_1 + 1)) {
      param = i_1;
      const float x_55 = func_i1_(param);
      param_1 = i_1;
      const float x_57 = func_i1_(param_1);
      if ((x_57 == 1.0f)) {
        x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
      } else {
        x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
      }
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
  const main_out tint_symbol_3 = {x_GLF_color};
  return tint_symbol_3;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
