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
  a = asint(x_6[0u].x);
  v = float4((asfloat(x_9[2u].x)).xxxx);
  float x_49 = asfloat(x_9[3u].x);
  float4 v_1 = float4(x_49, 0.0f, 0.0f, 0.0f);
  float4 v_2 = float4(0.0f, x_49, 0.0f, 0.0f);
  m = float3x4(v_1, v_2, float4(0.0f, 0.0f, x_49, 0.0f));
  int x_54 = a;
  int x_55 = a;
  m[x_54][x_55] = asfloat(x_9[0u].x);
  int x_59 = a;
  int x_78 = a;
  int x_79 = a;
  float4 v_3 = float4(m[0u].x, m[0u].y, m[0u].z, m[0u].w);
  float4 v_4 = float4(m[1u].x, m[1u].y, m[1u].z, m[1u].w);
  indexable = float4x4(v_3, v_4, float4(m[2u].x, m[2u].y, m[2u].z, m[2u].w), float4(0.0f, 0.0f, 0.0f, 1.0f));
  v[x_59] = (v[x_59] + indexable[x_78][x_79]);
  float v_5 = v.y;
  if ((v_5 == asfloat(x_9[1u].x))) {
    float v_6 = float(asint(x_6[0u].x));
    float v_7 = float(asint(x_6[1u].x));
    float v_8 = float(asint(x_6[1u].x));
    x_GLF_color = float4(v_6, v_7, v_8, float(asint(x_6[0u].x)));
  } else {
    x_GLF_color = float4((float(asint(x_6[1u].x))).xxxx);
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
<scrubbed_path>(30,3-9): error X3500: array reference cannot be used as an l-value; not natively addressable


tint executable returned error: exit status 1
