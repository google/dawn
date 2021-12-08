SKIP: FAILED

struct Array {
  int values[2];
};

cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  Array a = (Array)0;
  Array b = (Array)0;
  float one = 0.0f;
  const int x_10 = asint(x_7[0].x);
  a.values[x_10] = 1;
  b = a;
  one = 0.0f;
  const int x_11 = asint(x_7[0].x);
  const int x_12 = b.values[x_11];
  if ((x_12 == 1)) {
    one = 1.0f;
  }
  x_GLF_color = float4(one, 0.0f, 0.0f, 1.0f);
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
C:\src\tint\test\Shader@0x0000024C52B7BDB0(15,3-16): error X3500: array reference cannot be used as an l-value; not natively addressable

