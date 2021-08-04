void set_float4(inout float4 vec, int idx, float val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int x = 0;
  float4 matrix_u = float4(0.0f, 0.0f, 0.0f, 0.0f);
  int b = 0;
  float4 matrix_b = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_42 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  x = 4;
  {
    for(; (x >= 1); x = (x - 1)) {
      set_float4(matrix_u, x, 1.0f);
    }
  }
  b = 4;
  while (true) {
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const float x_55 = asfloat(x_8[scalar_offset / 4][scalar_offset % 4]);
    if ((x_55 < -1.0f)) {
    } else {
      break;
    }
    const int x_14 = b;
    if ((b > 1)) {
      x_42 = min(matrix_b, matrix_b);
    } else {
      x_42 = matrix_u;
    }
    const float x_67 = x_42.y;
    set_float4(matrix_b, x_14, x_67);
    {
      b = (b - 1);
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
