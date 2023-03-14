diagnostic_filtering/switch_statement_attribute.wgsl:7:27 warning: 'dpdx' must only be called from uniform control flow
  switch (i32(x == 0.0 && dpdx(1.0) == 0.0)) {
                          ^^^^^^^^^

diagnostic_filtering/switch_statement_attribute.wgsl:7:24 note: control flow depends on possibly non-uniform value
  switch (i32(x == 0.0 && dpdx(1.0) == 0.0)) {
                       ^^

diagnostic_filtering/switch_statement_attribute.wgsl:7:15 note: user-defined input 'x' of 'main' may be non-uniform
  switch (i32(x == 0.0 && dpdx(1.0) == 0.0)) {
              ^

#version 310 es
precision highp float;

layout(location = 0) in float x_1;
void tint_symbol(float x) {
  bool tint_tmp = (x == 0.0f);
  if (tint_tmp) {
    tint_tmp = (dFdx(1.0f) == 0.0f);
  }
  switch(int((tint_tmp))) {
    default: {
      break;
    }
  }
}

void main() {
  tint_symbol(x_1);
  return;
}
