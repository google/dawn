void set_float4(inout float4 vec, int idx, float val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int k = 0;
  int GLF_dead0j = 0;
  int donor_replacementGLF_dead0stack[10] = (int[10])0;
  int donor_replacementGLF_dead0top = 0;
  int x_54 = 0;
  float4 matrix_b = float4(0.0f, 0.0f, 0.0f, 0.0f);
  int b = 0;
  k = 0;
  {
    for(; (k < 4); k = (k + 1)) {
      const float x_62 = asfloat(x_6[0].y);
      if ((0.0f > x_62)) {
        GLF_dead0j = 1;
        while (true) {
          const int x_13 = donor_replacementGLF_dead0stack[0];
          if ((1 <= x_13)) {
          } else {
            break;
          }
        }
        if (((donor_replacementGLF_dead0top >= 0) & (donor_replacementGLF_dead0top < 9))) {
          const int x_17 = (donor_replacementGLF_dead0top + 1);
          donor_replacementGLF_dead0top = x_17;
          x_54 = x_17;
        } else {
          x_54 = 0;
        }
        donor_replacementGLF_dead0stack[x_54] = 1;
      }
      matrix_b = float4(0.0f, 0.0f, 0.0f, 0.0f);
      b = 3;
      {
        for(; (b >= 0); b = (b - 1)) {
          const int x_20 = b;
          const float x_87 = matrix_b[b];
          set_float4(matrix_b, x_20, (x_87 - 1.0f));
        }
      }
    }
  }
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
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
  const main_out tint_symbol_2 = {x_GLF_color};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
