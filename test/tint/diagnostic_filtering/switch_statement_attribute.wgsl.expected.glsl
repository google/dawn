<dawn>/test/tint/diagnostic_filtering/switch_statement_attribute.wgsl:7:27 warning: 'dpdx' must only be called from uniform control flow
  switch (i32(x == 0.0 && dpdx(1.0) == 0.0)) {
                          ^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/switch_statement_attribute.wgsl:7:15 note: control flow depends on possibly non-uniform value
  switch (i32(x == 0.0 && dpdx(1.0) == 0.0)) {
              ^^^^^^^^^^^^^^^^^^^^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/switch_statement_attribute.wgsl:7:15 note: user-defined input 'x' of 'main' may be non-uniform
  switch (i32(x == 0.0 && dpdx(1.0) == 0.0)) {
              ^

#version 310 es
precision highp float;
precision highp int;

layout(location = 0) in float x_1;
void tint_symbol(float x) {
  bool tint_symbol_1 = (x == 0.0f);
  if (tint_symbol_1) {
    float tint_symbol_2 = dFdx(1.0f);
    tint_symbol_1 = (tint_symbol_2 == 0.0f);
  }
  switch(int(tint_symbol_1)) {
    default: {
      break;
    }
  }
}

void main() {
  tint_symbol(x_1);
  return;
}
