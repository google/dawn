SKIP: FAILED

static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float func_f1_(inout float x) {
  const float x_56 = x;
  if ((x_56 > 5.0f)) {
    const float x_61 = gl_FragCoord.x;
    if ((x_61 < 0.5f)) {
      discard;
    } else {
      const float x_67 = gl_FragCoord.y;
      if ((x_67 < 0.5f)) {
        discard;
      }
    }
  }
  const float x_71 = x;
  return (x_71 + 1.0f);
}

void main_1() {
  float f = 0.0f;
  int i = 0;
  float param = 0.0f;
  f = 0.0f;
  i = 0;
  while (true) {
    const int x_39 = i;
    const int x_41 = asint(x_10[0].x);
    if ((x_39 < x_41)) {
    } else {
      break;
    }
    {
      param = float(i);
      const float x_47 = func_f1_(param);
      f = x_47;
      i = (i + 1);
    }
  }
  if ((f == 5.0f)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
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

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const float4 gl_FragCoord_param = tint_symbol.gl_FragCoord_param;
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_3 = {x_GLF_color};
  const tint_symbol_2 tint_symbol_5 = {tint_symbol_3.x_GLF_color_1};
  return tint_symbol_5;
}
C:\src\tint\test\Shader@0x000001976079AA90(7,14-29): error X3507: 'func_f1_': Not all control paths return a value

