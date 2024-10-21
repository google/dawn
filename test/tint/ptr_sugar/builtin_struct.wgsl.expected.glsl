#version 310 es


struct modf_result_f32 {
  float fract;
  float whole;
};

struct frexp_result_f32 {
  float fract;
  int exp;
};

void deref_modf() {
  modf_result_f32 a = modf_result_f32(0.5f, 1.0f);
  float tint_symbol = a.fract;
  float whole = a.whole;
}
void no_deref_modf() {
  modf_result_f32 a = modf_result_f32(0.5f, 1.0f);
  float tint_symbol = a.fract;
  float whole = a.whole;
}
void deref_frexp() {
  frexp_result_f32 a = frexp_result_f32(0.75f, 1);
  float tint_symbol = a.fract;
  int tint_symbol_1 = a.exp;
}
void no_deref_frexp() {
  frexp_result_f32 a = frexp_result_f32(0.75f, 1);
  float tint_symbol = a.fract;
  int tint_symbol_1 = a.exp;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  deref_modf();
  no_deref_modf();
  deref_frexp();
  no_deref_frexp();
}
