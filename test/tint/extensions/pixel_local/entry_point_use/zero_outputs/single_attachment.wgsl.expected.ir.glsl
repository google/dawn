SKIP: INVALID

struct PixelLocal {
  uint a;
};
precision highp float;
precision highp int;


PixelLocal P;
void main() {
  P.a = (P.a + 42u);
}
