SKIP: FAILED

cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int i = 0;
  int value = 0;
  float y = 0.0f;
  int x_31_phi = 0;
  i = 0;
  x_31_phi = 0;
  while (true) {
    const int x_31 = x_31_phi;
    const float x_37 = asfloat(x_6[0].x);
    if ((x_31 < (2 + int(x_37)))) {
    } else {
      break;
    }
    float x_55_phi = 0.0f;
    float x_46_phi = 0.0f;
    value = x_31;
    y = 0.5f;
    x_55_phi = 0.5f;
    x_46_phi = 0.5f;
    switch(x_31) {
      case 0: {
        const float x_54 = (0.5f + 0.5f);
        y = x_54;
        x_55_phi = x_54;
        /* fallthrough */
      }
      case 1: {
        const float x_47 = clamp(1.0f, 0.5f, x_55_phi);
        y = x_47;
        x_46_phi = x_47;
        /* fallthrough */
      }
      default: {
        /* fallthrough */
      }
      case 2: {
        if ((x_46_phi == 1.0f)) {
          x_GLF_color = float4(float((x_31 + 1)), 0.0f, 0.0f, 1.0f);
          return;
        }
        break;
      }
    }
    {
      const int x_32 = (x_31 + 1);
      i = x_32;
      x_31_phi = x_32;
    }
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

tint_symbol main() {
  main_1();
  const main_out tint_symbol_1 = {x_GLF_color};
  const tint_symbol tint_symbol_3 = {tint_symbol_1.x_GLF_color_1};
  return tint_symbol_3;
}
C:\src\tint\test\Shader@0x000002EAE8878270(27,7): error X3533: non-empty case statements must have break or return
C:\src\tint\test\Shader@0x000002EAE8878270(33,7): error X3533: non-empty case statements must have break or return
C:\src\tint\test\Shader@0x000002EAE8878270(39,7): error X3533: non-empty case statements must have break or return
C:\src\tint\test\Shader@0x000002EAE8878270(32,7): error X3537: Fall-throughs in switch statements are not allowed.
C:\src\tint\test\Shader@0x000002EAE8878270(38,7): error X3537: Fall-throughs in switch statements are not allowed.
C:\src\tint\test\Shader@0x000002EAE8878270(41,7): error X3537: Fall-throughs in switch statements are not allowed.

