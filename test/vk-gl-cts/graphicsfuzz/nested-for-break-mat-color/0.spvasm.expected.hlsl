SKIP: FAILED

void set_float4(inout float4 vec, int idx, float val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4x4 m44 = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int x_10_phi = 0;
  m44 = float4x4(float4(1.0f, 2.0f, 3.0f, 4.0f), float4(5.0f, 6.0f, 7.0f, 8.0f), float4(9.0f, 10.0f, 11.0f, 12.0f), float4(13.0f, 14.0f, 15.0f, 16.0f));
  x_10_phi = 0;
  while (true) {
    int x_9 = 0;
    int x_11_phi = 0;
    const int x_10 = x_10_phi;
    if ((x_10 < 4)) {
    } else {
      break;
    }
    const float x_63 = gl_FragCoord.y;
    if ((x_63 < 0.0f)) {
      break;
    }
    x_11_phi = 0;
    while (true) {
      int x_8 = 0;
      const int x_11 = x_11_phi;
      if ((x_11 < 4)) {
      } else {
        break;
      }
      {
        const float x_72 = asfloat(x_7[0].x);
        const int x_73_save = x_10;
        const int x_73_save_1 = x_11;
        const float x_74 = m44[x_73_save][x_73_save_1];
        set_float4(m44[x_73_save], x_73_save_1, (x_74 + x_72));
        x_8 = (x_11 + 1);
        x_11_phi = x_8;
      }
    }
    {
      x_9 = (x_10 + 1);
      x_10_phi = x_9;
    }
  }
  const float x_77 = m44[1].y;
  float4 x_79_1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  x_79_1.x = (x_77 - 6.0f);
  const float4 x_79 = x_79_1;
  const float x_81 = m44[2].z;
  float4 x_83_1 = x_79;
  x_83_1.w = (x_81 - 11.0f);
  x_GLF_color = x_83_1;
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
C:\src\tint\test\Shader@0x0000018B52EEB9C0(16,3-14): error X3511: forced to unroll loop, but unrolling failed.

