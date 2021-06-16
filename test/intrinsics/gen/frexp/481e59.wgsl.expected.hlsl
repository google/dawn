void frexp_481e59() {
  int arg_1 = 0;
  float tint_tmp;
  float tint_tmp_1 = frexp(1.0f, tint_tmp);
  arg_1 = int(tint_tmp);
  float res = tint_tmp_1;
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  frexp_481e59();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  frexp_481e59();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_481e59();
  return;
}
