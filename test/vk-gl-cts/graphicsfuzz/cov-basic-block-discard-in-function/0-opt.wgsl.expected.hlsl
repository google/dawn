SKIP: FAILED

cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

int func_i1_(inout int x) {
  const int x_45 = x;
  if ((x_45 == 10)) {
    discard;
  }
  const int x_49 = x;
  return x_49;
}

void main_1() {
  int a = 0;
  int b = 0;
  int param = 0;
  int x_37 = 0;
  int x_35_phi = 0;
  a = 0;
  const int x_33 = asint(x_9[0].x);
  b = x_33;
  x_35_phi = x_33;
  while (true) {
    const int x_35 = x_35_phi;
    param = x_35;
    x_37 = func_i1_(param);
    a = x_37;
    const int x_36 = (x_35 + 1);
    b = x_36;
    x_35_phi = x_36;
    if ((x_36 < 4)) {
    } else {
      break;
    }
  }
  if ((x_37 == asint(3))) {
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

tint_symbol main() {
  main_1();
  const main_out tint_symbol_1 = {x_GLF_color};
  const tint_symbol tint_symbol_3 = {tint_symbol_1.x_GLF_color_1};
  return tint_symbol_3;
}
C:\src\tint\test\Shader@0x0000026EBFC3A3B0(6,12-25): error X3507: 'func_i1_': Not all control paths return a value

