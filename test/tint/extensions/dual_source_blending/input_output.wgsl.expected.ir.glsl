SKIP: FAILED

#version 310 es

struct FragOutput {
  vec4 color;
  vec4 blend;
};
precision highp float;
precision highp int;


struct FragInput {
  vec4 a;
  vec4 b;
};

FragOutput main(FragInput tint_symbol) {
  FragOutput tint_symbol_1 = FragOutput(vec4(0.0f), vec4(0.0f));
  tint_symbol_1.color = tint_symbol.a;
  tint_symbol_1.blend = tint_symbol.b;
  return tint_symbol_1;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
