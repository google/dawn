SKIP: FAILED

static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0) {
  uint4 x_6[1];
};

void main_1() {
  int GLF_dead6index = 0;
  int GLF_dead6currentNode = 0;
  int donor_replacementGLF_dead6tree[1] = (int[1])0;
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  GLF_dead6index = 0;
  const float x_34 = asfloat(x_6[0].y);
  if ((x_34 < 0.0f)) {
    while (true) {
      if (true) {
      } else {
        break;
      }
      const int x_9 = GLF_dead6index;
      const int x_10 = donor_replacementGLF_dead6tree[x_9];
      GLF_dead6currentNode = x_10;
      const int x_11 = GLF_dead6currentNode;
      GLF_dead6index = x_11;
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

main_out main_inner() {
  main_1();
  const main_out tint_symbol_1 = {x_GLF_color};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
