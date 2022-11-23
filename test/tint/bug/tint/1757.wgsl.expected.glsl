bug/tint/1757.wgsl:6:25 warning: use of deprecated language feature: 'sig' has been renamed to 'fract'
    let sig : f32 = res.sig;
                        ^^^

#version 310 es

struct frexp_result {
  float fract;
  int exp;
};


void tint_symbol() {
  frexp_result res = frexp_result(0.61500001f, 1);
  int tint_symbol_1 = res.exp;
  float sig = res.fract;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
