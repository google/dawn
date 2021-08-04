static int3 x_1 = int3(0, 0, 0);

void main_1() {
  const int3 x_2 = x_1;
  return;
}

struct tint_symbol_1 {
  uint3 x_1_param : SV_DispatchThreadID;
};

void main_inner(uint3 x_1_param) {
  x_1 = asint(x_1_param);
  main_1();
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.x_1_param);
  return;
}
