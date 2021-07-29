SKIP: FAILED

cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float data[2] = (float[2])0;
  float x_32 = 0.0f;
  x_32 = asfloat(x_6[0].x);
  data[0] = x_32;
  data[1] = x_32;
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  const float x_35 = data[1];
  if ((x_35 > 1.0f)) {
    float x_43_phi = 0.0f;
    const int x_39 = int(x_32);
    x_43_phi = 0.0f;
    switch(x_39) {
      case 0: {
        x_43_phi = 1.0f;
        /* fallthrough */
      }
      case 1: {
        data[x_39] = x_43_phi;
        x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
        break;
      }
      default: {
        break;
      }
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
C:\src\tint\test\Shader@0x0000020777179050(19,7): error X3533: non-empty case statements must have break or return
C:\src\tint\test\Shader@0x0000020777179050(22,7): error X3537: Fall-throughs in switch statements are not allowed.

