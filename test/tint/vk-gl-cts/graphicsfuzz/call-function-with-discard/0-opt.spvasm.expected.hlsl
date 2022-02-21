SKIP: FAILED

cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void func_() {
  const float x_28 = asfloat(x_6[0].x);
  if ((1.0f > x_28)) {
    discard;
  }
  return;
}

void main_1() {
  x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  [loop] while (true) {
    func_();
    if (false) {
    } else {
      break;
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
C:\src\tint\test\Shader@0x0000016102DC2B30(16,10-21): warning X3557: loop only executes for 0 iteration(s), consider removing [loop]
C:\src\tint\test\Shader@0x0000016102DC2B30(8,15-18): internal error: invalid access of unbound variable

