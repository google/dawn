SKIP: FAILED

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


cbuffer cbuffer_x_6 : register(b0) {
  uint4 x_6[4];
};
cbuffer cbuffer_x_9 : register(b1) {
  uint4 x_9[2];
};
static float4 x_GLF_color = (0.0f).xxxx;
void main_1() {
  float3x3 m = float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  int a = 0;
  float3 arr[2] = (float3[2])0;
  float3 v = (0.0f).xxx;
  float x_46 = float(asint(x_6[0u].x));
  float3 v_1 = float3(x_46, 0.0f, 0.0f);
  float3 v_2 = float3(0.0f, x_46, 0.0f);
  m = float3x3(v_1, v_2, float3(0.0f, 0.0f, x_46));
  a = asint(x_6[0u].x);
  int x_53 = a;
  int x_54 = a;
  m[x_53][x_54] = asfloat(x_9[0u].x);
  float3 v_3[2] = {m[1], m[1]};
  arr = v_3;
  v = float3((asfloat(x_9[1u].x)).xxx);
  v = (v + arr[a]);
  float3 v_4 = v;
  float v_5 = float(asint(x_6[1u].x));
  float v_6 = float(asint(x_6[2u].x));
  if (all((v_4 == float3(v_5, v_6, float(asint(x_6[1u].x)))))) {
    float v_7 = float(asint(x_6[0u].x));
    float v_8 = float(asint(x_6[3u].x));
    float v_9 = float(asint(x_6[3u].x));
    x_GLF_color = float4(v_7, v_8, v_9, float(asint(x_6[0u].x)));
  } else {
    x_GLF_color = float4((float(asint(x_6[3u].x))).xxxx);
  }
}

main_out main_inner() {
  main_1();
  main_out v_10 = {x_GLF_color};
  return v_10;
}

main_outputs main() {
  main_out v_11 = main_inner();
  main_outputs v_12 = {v_11.x_GLF_color_1};
  return v_12;
}

FXC validation failure:
<scrubbed_path>(29,3-9): error X3500: array reference cannot be used as an l-value; not natively addressable


tint executable returned error: exit status 1
