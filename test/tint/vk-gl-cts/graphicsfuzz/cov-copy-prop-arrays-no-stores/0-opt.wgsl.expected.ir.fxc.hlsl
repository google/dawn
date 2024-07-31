SKIP: FAILED

struct Array {
  int values[2];
};

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


cbuffer cbuffer_x_7 : register(b0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = (0.0f).xxxx;
void main_1() {
  Array a = (Array)0;
  Array b = (Array)0;
  float one = 0.0f;
  int x_10 = asint(x_7[0u].x);
  a.values[x_10] = 1;
  Array v = a;
  Array x_35 = v;
  b = x_35;
  one = 0.0f;
  int x_11 = asint(x_7[0u].x);
  int x_12 = b.values[x_11];
  if ((x_12 == 1)) {
    one = 1.0f;
  }
  float x_41 = one;
  x_GLF_color = float4(x_41, 0.0f, 0.0f, 1.0f);
}

main_out main_inner() {
  main_1();
  main_out v_1 = {x_GLF_color};
  return v_1;
}

main_outputs main() {
  main_out v_2 = main_inner();
  main_outputs v_3 = {v_2.x_GLF_color_1};
  return v_3;
}

FXC validation failure:
c:\src\dawn\Shader@0x0000015F3932CAB0(23,3-16): error X3500: array reference cannot be used as an l-value; not natively addressable

