SKIP: FAILED

struct modf_result_vec2_f32 {
  float2 fract;
  float2 whole;
};


[numthreads(1, 1, 1)]
void main() {
  float2 tint_symbol = float2(1.25f, 3.75f);
  modf_result_vec2_f32 v = modf(tint_symbol);
  modf_result_vec2_f32 res = v;
  float2 fract = res.fract;
  modf_result_vec2_f32 v_1 = v;
  float2 whole = v_1.whole;
}

DXC validation failure:
hlsl.hlsl:10:28: error: use of undeclared identifier 'modf'
  modf_result_vec2_f32 v = modf(tint_symbol);
                           ^


tint executable returned error: exit status 1
