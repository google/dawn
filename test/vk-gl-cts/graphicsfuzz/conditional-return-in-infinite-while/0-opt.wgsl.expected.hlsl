static int GLF_live6tree[10] = (int[10])0;
cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

int GLF_live6search_() {
  int GLF_live6index = 0;
  int GLF_live6currentNode = 0;
  GLF_live6index = 0;
  while (true) {
    if (true) {
    } else {
      break;
    }
    const int x_11 = GLF_live6tree[GLF_live6index];
    GLF_live6currentNode = x_11;
    if ((GLF_live6currentNode != 1)) {
      return 1;
    }
    GLF_live6index = 1;
  }
  return 1;
}

void main_1() {
  const float x_40 = asfloat(x_9[0].x);
  if ((x_40 > 1.0f)) {
    const int x_13 = GLF_live6search_();
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
