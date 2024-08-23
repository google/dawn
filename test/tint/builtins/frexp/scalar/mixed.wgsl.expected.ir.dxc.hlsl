SKIP: FAILED

struct frexp_result_f32 {
  float fract;
  int exp;
};


[numthreads(1, 1, 1)]
void main() {
  float runtime_in = 1.25f;
  frexp_result_f32 res = {0.625f, 1};
  frexp_result_f32 v = frexp(runtime_in);
  res = v;
  frexp_result_f32 v_1 = {0.625f, 1};
  res = v_1;
  float fract = res.fract;
  int exp = res.exp;
}

DXC validation failure:
hlsl.hlsl:11:24: error: use of undeclared identifier 'frexp'
  frexp_result_f32 v = frexp(runtime_in);
                       ^


tint executable returned error: exit status 1
