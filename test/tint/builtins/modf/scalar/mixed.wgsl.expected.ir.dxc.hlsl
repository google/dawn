SKIP: FAILED

struct modf_result_f32 {
  float fract;
  float whole;
};


[numthreads(1, 1, 1)]
void main() {
  float runtime_in = 1.25f;
  modf_result_f32 res = {0.25f, 1.0f};
  modf_result_f32 v = modf(runtime_in);
  res = v;
  modf_result_f32 v_1 = {0.25f, 1.0f};
  res = v_1;
  float fract = res.fract;
  float whole = res.whole;
}

DXC validation failure:
hlsl.hlsl:11:23: error: use of undeclared identifier 'modf'
  modf_result_f32 v = modf(runtime_in);
                      ^


tint executable returned error: exit status 1
