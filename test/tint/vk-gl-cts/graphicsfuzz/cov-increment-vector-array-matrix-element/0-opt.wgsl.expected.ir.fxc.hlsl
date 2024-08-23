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
  int x_45 = asint(x_6[0u].x);
  float x_46 = float(x_45);
  float3 v_1 = float3(x_46, 0.0f, 0.0f);
  float3 v_2 = float3(0.0f, x_46, 0.0f);
  m = float3x3(v_1, v_2, float3(0.0f, 0.0f, x_46));
  int x_52 = asint(x_6[0u].x);
  a = x_52;
  int x_53 = a;
  int x_54 = a;
  float x_56 = asfloat(x_9[0u].x);
  m[x_53][x_54] = x_56;
  float3 x_59 = m[1];
  float3 x_61 = m[1];
  float3 v_3[2] = {x_59, x_61};
  arr = v_3;
  float x_64 = asfloat(x_9[1u].x);
  v = float3(x_64, x_64, x_64);
  int x_66 = a;
  float3 x_68 = arr[x_66];
  float3 x_69 = v;
  v = (x_69 + x_68);
  float3 x_71 = v;
  int x_73 = asint(x_6[1u].x);
  int x_76 = asint(x_6[2u].x);
  int x_79 = asint(x_6[1u].x);
  float v_4 = float(x_73);
  float v_5 = float(x_76);
  if (all((x_71 == float3(v_4, v_5, float(x_79))))) {
    int x_88 = asint(x_6[0u].x);
    int x_91 = asint(x_6[3u].x);
    int x_94 = asint(x_6[3u].x);
    int x_97 = asint(x_6[0u].x);
    float v_6 = float(x_88);
    float v_7 = float(x_91);
    float v_8 = float(x_94);
    x_GLF_color = float4(v_6, v_7, v_8, float(x_97));
  } else {
    int x_101 = asint(x_6[3u].x);
    float x_102 = float(x_101);
    x_GLF_color = float4(x_102, x_102, x_102, x_102);
  }
}

main_out main_inner() {
  main_1();
  main_out v_9 = {x_GLF_color};
  return v_9;
}

main_outputs main() {
  main_out v_10 = main_inner();
  main_outputs v_11 = {v_10.x_GLF_color_1};
  return v_11;
}

FXC validation failure:
<scrubbed_path>(32,3-9): error X3500: array reference cannot be used as an l-value; not natively addressable


tint executable returned error: exit status 1
