SKIP: FAILED

struct modf_result_f32 {
  float fract;
  float whole;
};


[numthreads(1, 1, 1)]
void main() {
  float tint_symbol = 1.25f;
  modf_result_f32 v = modf(tint_symbol);
  modf_result_f32 res = v;
  float fract = res.fract;
  modf_result_f32 v_1 = v;
  float whole = v_1.whole;
}

DXC validation failure:
hlsl.hlsl:10:23: error: use of undeclared identifier 'modf'
  modf_result_f32 v = modf(tint_symbol);
                      ^


tint executable returned error: exit status 1
