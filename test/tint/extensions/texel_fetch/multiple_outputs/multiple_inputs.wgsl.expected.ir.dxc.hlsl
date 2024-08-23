SKIP: FAILED

struct Out {
  float4 x;
  float4 y;
  float4 z;
};

struct f_outputs {
  float4 Out_x : SV_Target0;
  float4 Out_y : SV_Target2;
  float4 Out_z : SV_Target4;
};

struct f_inputs {
  float4 fbf_1;
  float4 fbf_3;
};


Out f_inner(float4 fbf_1, float4 fbf_3) {
  Out v = {fbf_1, (20.0f).xxxx, fbf_3};
  return v;
}

f_outputs f(f_inputs inputs) {
  Out v_1 = f_inner(inputs.fbf_1, inputs.fbf_3);
  Out v_2 = v_1;
  Out v_3 = v_1;
  Out v_4 = v_1;
  f_outputs v_5 = {v_2.x, v_3.y, v_4.z};
  return v_5;
}

DXC validation failure:
hlsl.hlsl:24:1: error: Semantic must be defined for all parameters of an entry function or patch constant function
f_outputs f(f_inputs inputs) {
^
hlsl.hlsl:24:1: error: Semantic must be defined for all parameters of an entry function or patch constant function


tint executable returned error: exit status 1
