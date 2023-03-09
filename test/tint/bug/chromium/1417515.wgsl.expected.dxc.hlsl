struct modf_result_f32 {
  float fract;
  float whole;
};
[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void foo() {
  const modf_result_f32 s1 = (modf_result_f32)0;
}
