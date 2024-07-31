struct modf_result_f32 {
  float fract;
  float whole;
};

struct frexp_result_f32 {
  float fract;
  int exp;
};


void deref_modf() {
  modf_result_f32 a = {0.5f, 1.0f};
  modf_result_f32 p = a;
  float fract = p.fract;
  float whole = p.whole;
}

void no_deref_modf() {
  modf_result_f32 a = {0.5f, 1.0f};
  modf_result_f32 p = a;
  float fract = p.fract;
  float whole = p.whole;
}

void deref_frexp() {
  frexp_result_f32 a = {0.75f, 1};
  frexp_result_f32 p = a;
  float fract = p.fract;
  int exp = p.exp;
}

void no_deref_frexp() {
  frexp_result_f32 a = {0.75f, 1};
  frexp_result_f32 p = a;
  float fract = p.fract;
  int exp = p.exp;
}

[numthreads(1, 1, 1)]
void main() {
  deref_modf();
  no_deref_modf();
  deref_frexp();
  no_deref_frexp();
}

