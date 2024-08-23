SKIP: FAILED

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


cbuffer cbuffer_x_6 : register(b0) {
  uint4 x_6[2];
};
cbuffer cbuffer_x_9 : register(b1) {
  uint4 x_9[4];
};
static float4 x_GLF_color = (0.0f).xxxx;
void main_1() {
  int a = 0;
  float4 v = (0.0f).xxxx;
  float3x4 m = float3x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
  float4x4 indexable = float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
  int x_44 = asint(x_6[0u].x);
  a = x_44;
  float x_46 = asfloat(x_9[2u].x);
  v = float4(x_46, x_46, x_46, x_46);
  float x_49 = asfloat(x_9[3u].x);
  float4 v_1 = float4(x_49, 0.0f, 0.0f, 0.0f);
  float4 v_2 = float4(0.0f, x_49, 0.0f, 0.0f);
  m = float3x4(v_1, v_2, float4(0.0f, 0.0f, x_49, 0.0f));
  int x_54 = a;
  int x_55 = a;
  float x_57 = asfloat(x_9[0u].x);
  m[x_54][x_55] = x_57;
  int x_59 = a;
  float3x4 x_60 = m;
  int x_78 = a;
  int x_79 = a;
  float4 v_3 = float4(x_60[0u][0u], x_60[0u][1u], x_60[0u][2u], x_60[0u][3u]);
  float4 v_4 = float4(x_60[1u][0u], x_60[1u][1u], x_60[1u][2u], x_60[1u][3u]);
  indexable = float4x4(v_3, v_4, float4(x_60[2u][0u], x_60[2u][1u], x_60[2u][2u], x_60[2u][3u]), float4(0.0f, 0.0f, 0.0f, 1.0f));
  float x_81 = indexable[x_78][x_79];
  float x_83 = v[x_59];
  v[x_59] = (x_83 + x_81);
  float x_87 = v.y;
  float x_89 = asfloat(x_9[1u].x);
  if ((x_87 == x_89)) {
    int x_95 = asint(x_6[0u].x);
    int x_98 = asint(x_6[1u].x);
    int x_101 = asint(x_6[1u].x);
    int x_104 = asint(x_6[0u].x);
    float v_5 = float(x_95);
    float v_6 = float(x_98);
    float v_7 = float(x_101);
    x_GLF_color = float4(v_5, v_6, v_7, float(x_104));
  } else {
    int x_108 = asint(x_6[1u].x);
    float x_109 = float(x_108);
    x_GLF_color = float4(x_109, x_109, x_109, x_109);
  }
}

main_out main_inner() {
  main_1();
  main_out v_8 = {x_GLF_color};
  return v_8;
}

main_outputs main() {
  main_out v_9 = main_inner();
  main_outputs v_10 = {v_9.x_GLF_color_1};
  return v_10;
}

FXC validation failure:
<scrubbed_path>(33,3-9): error X3500: array reference cannot be used as an l-value; not natively addressable


tint executable returned error: exit status 1
