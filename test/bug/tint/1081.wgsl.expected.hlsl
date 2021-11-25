bug/tint/1081.wgsl:9:25 warning: integral user-defined fragment inputs must have a flat interpolation attribute
fn main([[location(1)]] x: vec3<i32>) -> [[location(2)]] i32 {
                        ^

int f(int x) {
  if (true) {
    if ((x == 10)) {
      discard;
    }
    return x;
  }
  int unused;
  return unused;
}

struct tint_symbol_1 {
  int3 x : TEXCOORD1;
};
struct tint_symbol_2 {
  int value : SV_Target2;
};

int main_inner(int3 x) {
  int y = x.x;
  [loop] while (true) {
    const int r = f(y);
    if ((r == 0)) {
      break;
    }
  }
  return y;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const int inner_result = main_inner(tint_symbol.x);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
