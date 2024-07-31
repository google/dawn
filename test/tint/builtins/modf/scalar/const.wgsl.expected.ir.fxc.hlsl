struct modf_result_f32 {
  float fract;
  float whole;
};


[numthreads(1, 1, 1)]
void main() {
  modf_result_f32 v = {0.25f, 1.0f};
  modf_result_f32 res = v;
  float fract = res.fract;
  modf_result_f32 v_1 = v;
  float whole = v_1.whole;
}

