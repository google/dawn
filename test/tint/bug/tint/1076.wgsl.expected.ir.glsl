SKIP: FAILED

#version 310 es

struct FragIn {
  float a;
  uint mask;
};
precision highp float;
precision highp int;


FragIn main(FragIn tint_symbol_1, float b) {
  if ((tint_symbol_1.mask == 0u)) {
    return tint_symbol_1;
  }
  return FragIn(b, 1u);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
