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
  v1 = float2((x_35).xx);
  int x_38 = asint(x_8[0u].x);
  v1[x_38] = ldexp(v1.y, -256);
  float2 v = v1;
  float2 v_1 = float2(x_35, 0.0f);
  if ((mul(float2x2(v_1, float2(0.0f, x_35)), v)[0u] == x_35)) {
    float x_53 = float(x_38);
    float x_56 = float(asint(x_8[1u].x));
    x_GLF_color = float4(x_53, x_56, x_56, x_53);
  } else {
    x_GLF_color = float4((float(asint(x_8[1u].x))).xxxx);
  }
}

main_out main_inner() {
  main_1();
  main_out v_2 = {x_GLF_color};
  return v_2;
}

main_outputs main() {
  main_out v_3 = main_inner();
  main_outputs v_4 = {v_3.x_GLF_color_1};
  return v_4;
}

FXC validation failure:
<scrubbed_path>(22,3-10): error X3500: array reference cannot be used as an l-value; not natively addressable


tint executable returned error: exit status 1
