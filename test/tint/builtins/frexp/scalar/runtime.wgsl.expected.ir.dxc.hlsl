SKIP: FAILED

struct frexp_result_f32 {
  float fract;
  int exp;
};


[numthreads(1, 1, 1)]
void main() {
  float tint_symbol = 1.25f;
  frexp_result_f32 v = frexp(tint_symbol);
  frexp_result_f32 res = v;
  float fract = res.fract;
  frexp_result_f32 v_1 = v;
  int exp = v_1.exp;
}

DXC validation failure:
hlsl.hlsl:10:24: error: use of undeclared identifier 'frexp'
  frexp_result_f32 v = frexp(tint_symbol);
                       ^


tint executable returned error: exit status 1
