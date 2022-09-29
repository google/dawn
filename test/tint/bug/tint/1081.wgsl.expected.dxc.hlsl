static bool tint_discard = false;

int f(int x) {
  if ((x == 10)) {
    tint_discard = true;
    return 0;
  }
  return x;
}

struct tint_symbol_1 {
  nointerpolation int3 x : TEXCOORD1;
};
struct tint_symbol_2 {
  int value : SV_Target2;
};

int main_inner(int3 x) {
  int y = x.x;
  while (true) {
    const int r = f(y);
    if (tint_discard) {
      return 0;
    }
    if ((r == 0)) {
      break;
    }
  }
  return y;
}

void tint_discard_func() {
  discard;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const int inner_result = main_inner(tint_symbol.x);
  if (tint_discard) {
    tint_discard_func();
    const tint_symbol_2 tint_symbol_3 = (tint_symbol_2)0;
    return tint_symbol_3;
  }
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
