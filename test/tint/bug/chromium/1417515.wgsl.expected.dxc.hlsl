struct modf_result_f32 {
  float fract;
  float whole;
};


[numthreads(1, 1, 1)]
void foo() {
  modf_result_f32 s1 = (modf_result_f32)0;
}

