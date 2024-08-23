SKIP: FAILED

struct Out {
  float4 x;
  float4 y;
  float4 z;
};

struct f_outputs {
  float4 Out_x : SV_Target0;
  float4 Out_y : SV_Target2;
  float4 Out_z : SV_Target3;
};

struct f_inputs {
  float4 fbf;
};


Out f_inner(float4 fbf) {
  Out v = {(10.0f).xxxx, fbf, (30.0f).xxxx};
  return v;
}

f_outputs f(f_inputs inputs) {
  Out v_1 = f_inner(inputs.fbf);
  Out v_2 = v_1;
  Out v_3 = v_1;
  Out v_4 = v_1;
  f_outputs v_5 = {v_2.x, v_3.y, v_4.z};
  return v_5;
}

DXC validation failure:
hlsl.hlsl:23:1: error: Semantic must be defined for all parameters of an entry function or patch constant function
f_outputs f(f_inputs inputs) {
^


tint executable returned error: exit status 1
