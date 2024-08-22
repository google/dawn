SKIP: FAILED

struct S {
  float numbers[3];
};

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


cbuffer cbuffer_x_7 : register(b1) {
  uint4 x_7[5];
};
cbuffer cbuffer_x_9 : register(b2) {
  uint4 x_9[1];
};
cbuffer cbuffer_x_12 : register(b3) {
  uint4 x_12[1];
};
cbuffer cbuffer_x_15 : register(b0) {
  uint4 x_15[2];
};
static float4 x_GLF_color = (0.0f).xxxx;
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (-2147483648))) : (2147483647));
}

void main_1() {
  S obj = (S)0;
  float a = 0.0f;
  float2 x_49 = (0.0f).xx;
  float b = 0.0f;
  float x_51 = asfloat(x_7[3u].x);
  float x_53 = asfloat(x_7[2u].x);
  float x_55 = asfloat(x_7[4u].x);
  float v[3] = {x_51, x_53, x_55};
  S v_1 = {v};
  obj = v_1;
  float x_59 = asfloat(x_9[0u].x);
  float x_62 = asfloat(x_7[0u].x);
  obj.numbers[tint_f32_to_i32(x_59)] = x_62;
  float x_65 = asfloat(x_9[0u].x);
  float x_67 = asfloat(x_7[0u].x);
  if ((x_65 > x_67)) {
    float2 x_73 = asfloat(x_9[0u].xy);
    x_49 = x_73;
  } else {
    float2 x_75 = asfloat(x_12[0u].xy);
    x_49 = x_75;
  }
  float x_77 = x_49.y;
  a = x_77;
  float x_79 = asfloat(x_7[0u].x);
  float x_80 = a;
  int x_82 = asint(x_15[0u].x);
  float x_84 = obj.numbers[x_82];
  b = lerp(x_79, x_80, x_84);
  float x_86 = b;
  float x_88 = asfloat(x_7[2u].x);
  float x_91 = asfloat(x_7[1u].x);
  if ((distance(x_86, x_88) < x_91)) {
    int x_97 = asint(x_15[0u].x);
    int x_100 = asint(x_15[1u].x);
    int x_103 = asint(x_15[1u].x);
    int x_106 = asint(x_15[0u].x);
    float v_2 = float(x_97);
    float v_3 = float(x_100);
    float v_4 = float(x_103);
    x_GLF_color = float4(v_2, v_3, v_4, float(x_106));
  } else {
    int x_110 = asint(x_15[1u].x);
    float x_111 = float(x_110);
    x_GLF_color = float4(x_111, x_111, x_111, x_111);
  }
}

main_out main_inner() {
  main_1();
  main_out v_5 = {x_GLF_color};
  return v_5;
}

main_outputs main() {
  main_out v_6 = main_inner();
  main_outputs v_7 = {v_6.x_GLF_color_1};
  return v_7;
}

FXC validation failure:
<scrubbed_path>(44,3-36): error X3500: array reference cannot be used as an l-value; not natively addressable

