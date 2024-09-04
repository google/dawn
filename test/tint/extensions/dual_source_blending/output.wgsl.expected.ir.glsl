SKIP: FAILED

#version 310 es

struct FragOutput {
  vec4 color;
  vec4 blend;
};
precision highp float;
precision highp int;


FragOutput main() {
  FragOutput tint_symbol = FragOutput(vec4(0.0f), vec4(0.0f));
  tint_symbol.color = vec4(0.5f, 0.5f, 0.5f, 1.0f);
  tint_symbol.blend = vec4(0.5f, 0.5f, 0.5f, 1.0f);
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
