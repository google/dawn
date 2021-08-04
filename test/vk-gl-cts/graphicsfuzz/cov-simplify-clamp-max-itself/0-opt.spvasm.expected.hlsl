void set_int4(inout int4 vec, int idx, int val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int4 a = int4(0, 0, 0, 0);
  int i = 0;
  int sum = 0;
  a = int4(0, 0, 0, 0);
  i = 0;
  while (true) {
    const int x_40 = i;
    const int x_42 = asint(x_7[0].w);
    if ((x_40 < (x_42 + 1))) {
    } else {
      break;
    }
    const int x_46 = i;
    const int x_48 = asint(x_7[0].x);
    const uint scalar_offset = ((4u * uint(clamp(x_46, x_48, i)))) / 4;
    const int x_52 = asint(x_7[scalar_offset / 4][scalar_offset % 4]);
    if ((x_52 == 1)) {
      set_int4(a, i, 5);
    } else {
      set_int4(a, i, i);
    }
    {
      i = (i + 1);
    }
  }
  const int x_65 = a.x;
  const int x_67 = a.y;
  const int x_70 = a.z;
  const int x_73 = a.w;
  sum = (((x_65 + x_67) + x_70) + x_73);
  if ((sum == 10)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
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
  const main_out tint_symbol_2 = {x_GLF_color};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
