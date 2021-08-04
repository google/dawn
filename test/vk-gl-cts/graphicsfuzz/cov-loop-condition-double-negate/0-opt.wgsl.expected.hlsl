cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[6];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int arr[3] = (int[3])0;
  int index = 0;
  bool x_76 = false;
  bool x_86 = false;
  bool x_77_phi = false;
  bool x_87_phi = false;
  const int x_33 = asint(x_6[3].x);
  const int x_35 = asint(x_6[5].x);
  const int x_37 = asint(x_6[2].x);
  const int tint_symbol_2[3] = {x_33, x_35, x_37};
  arr = tint_symbol_2;
  index = 1;
  while (true) {
    bool x_51 = false;
    bool x_52_phi = false;
    x_52_phi = true;
    if (true) {
      const uint scalar_offset = ((16u * uint(0))) / 4;
      const int x_46 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
      bool tint_tmp = (x_46 == 1);
      if (tint_tmp) {
        tint_tmp = (index <= 1);
      }
      x_51 = !((tint_tmp));
      x_52_phi = x_51;
    }
    if (!(x_52_phi)) {
    } else {
      break;
    }
    const int x_56_save = index;
    const int x_57 = arr[x_56_save];
    arr[x_56_save] = (x_57 + 1);
    index = (index + 1);
  }
  const int x_62 = asint(x_6[1].x);
  const int x_64 = arr[x_62];
  const int x_66 = asint(x_6[3].x);
  const bool x_67 = (x_64 == x_66);
  x_77_phi = x_67;
  if (x_67) {
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_71 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const int x_73 = arr[x_71];
    const int x_75 = asint(x_6[4].x);
    x_76 = (x_73 == x_75);
    x_77_phi = x_76;
  }
  const bool x_77 = x_77_phi;
  x_87_phi = x_77;
  if (x_77) {
    const int x_81 = asint(x_6[3].x);
    const int x_83 = arr[x_81];
    const int x_85 = asint(x_6[2].x);
    x_86 = (x_83 == x_85);
    x_87_phi = x_86;
  }
  if (x_87_phi) {
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_92 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const int x_95 = asint(x_6[1].x);
    const int x_98 = asint(x_6[1].x);
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_101 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    x_GLF_color = float4(float(x_92), float(x_95), float(x_98), float(x_101));
  } else {
    const int x_105 = asint(x_6[1].x);
    const float x_106 = float(x_105);
    x_GLF_color = float4(x_106, x_106, x_106, x_106);
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
  const main_out tint_symbol_3 = {x_GLF_color};
  return tint_symbol_3;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
