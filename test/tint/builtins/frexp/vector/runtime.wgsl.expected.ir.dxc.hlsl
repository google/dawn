SKIP: FAILED

struct frexp_result_vec2_f32 {
  float2 fract;
  int2 exp;
};


[numthreads(1, 1, 1)]
void main() {
  float2 tint_symbol = float2(1.25f, 3.75f);
  frexp_result_vec2_f32 v = frexp(tint_symbol);
  frexp_result_vec2_f32 res = v;
  float2 fract = res.fract;
  frexp_result_vec2_f32 v_1 = v;
  int2 exp = v_1.exp;
}

DXC validation failure:
hlsl.hlsl:10:29: error: use of undeclared identifier 'frexp'
  frexp_result_vec2_f32 v = frexp(tint_symbol);
                            ^


tint executable returned error: exit status 1
