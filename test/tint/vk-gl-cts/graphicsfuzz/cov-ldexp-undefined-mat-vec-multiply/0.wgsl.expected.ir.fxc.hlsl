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
cbuffer cbuffer_x_8 : register(b1) {
  uint4 x_8[2];
};
static float4 x_GLF_color = (0.0f).xxxx;
void main_1() {
  float2 v1 = (0.0f).xx;
  float x_35 = asfloat(x_6[0u].x);
  v1 = float2(x_35, x_35);
  int x_38 = asint(x_8[0u].x);
  float x_40 = v1.y;
  v1[x_38] = ldexp(x_40, -256);
  float2 x_43 = v1;
  float2 v = float2(x_35, 0.0f);
  if ((mul(float2x2(v, float2(0.0f, x_35)), x_43)[0u] == x_35)) {
    float x_53 = float(x_38);
    int x_55 = asint(x_8[1u].x);
    float x_56 = float(x_55);
    x_GLF_color = float4(x_53, x_56, x_56, x_53);
  } else {
    int x_59 = asint(x_8[1u].x);
    float x_60 = float(x_59);
    x_GLF_color = float4(x_60, x_60, x_60, x_60);
  }
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
<scrubbed_path>(23,3-10): error X3500: array reference cannot be used as an l-value; not natively addressable


tint executable returned error: exit status 1
