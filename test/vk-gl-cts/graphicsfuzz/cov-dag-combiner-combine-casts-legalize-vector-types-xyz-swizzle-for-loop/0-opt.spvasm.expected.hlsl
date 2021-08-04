void set_float4(inout float4 vec, int idx, float val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 v = float4(0.0f, 0.0f, 0.0f, 0.0f);
  int i = 0;
  const int x_36 = asint(x_6[3].x);
  const float x_37 = float(x_36);
  v = float4(x_37, x_37, x_37, x_37);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_40 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  i = x_40;
  while (true) {
    const int x_45 = i;
    const int x_47 = asint(x_6[3].x);
    if ((x_45 < x_47)) {
    } else {
      break;
    }
    set_float4(v, uint3(0u, 1u, 2u)[i], float(i));
    {
      i = (i + 1);
    }
  }
  const float4 x_57 = v;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_59 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  const int x_62 = asint(x_6[1].x);
  const int x_65 = asint(x_6[2].x);
  const int x_68 = asint(x_6[3].x);
  if (all((x_57 == float4(float(x_59), float(x_62), float(x_65), float(x_68))))) {
    const int x_77 = asint(x_6[1].x);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_80 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_83 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const int x_86 = asint(x_6[1].x);
    x_GLF_color = float4(float(x_77), float(x_80), float(x_83), float(x_86));
  } else {
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_90 = asint(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    const float x_91 = float(x_90);
    x_GLF_color = float4(x_91, x_91, x_91, x_91);
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
