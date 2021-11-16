SKIP: FAILED

bug/tint/1081.wgsl:9:25 warning: integral user-defined fragment inputs must have a flat interpolation attribute
fn main([[location(1)]] x: vec3<i32>) -> [[location(2)]] i32 {
                        ^

#version 310 es
precision mediump float;

int f(int x) {
  if ((x == 10)) {
    discard;
  }
  return x;
}

struct tint_symbol_2 {
  ivec3 x;
};
struct tint_symbol_3 {
  int value;
};

int tint_symbol_inner(ivec3 x) {
  int y = x.x;
  while (true) {
    int r = f(y);
    if ((r == 0)) {
      break;
    }
  }
  return y;
}

tint_symbol_3 tint_symbol(tint_symbol_2 tint_symbol_1) {
  int inner_result = tint_symbol_inner(tint_symbol_1.x);
  tint_symbol_3 wrapper_result = tint_symbol_3(0);
  wrapper_result.value = inner_result;
  return wrapper_result;
}
in ivec3 x;
out int value;
void main() {
  tint_symbol_2 inputs;
  inputs.x = x;
  tint_symbol_3 outputs;
  outputs = tint_symbol(inputs);
  value = outputs.value;
}


Error parsing GLSL shader:
ERROR: 0:35: 'int' : must be qualified as flat in
ERROR: 0:35: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



