SKIP: FAILED

#version 310 es

struct VSOut {
  vec4 pos;
};

void foo(inout VSOut tint_symbol) {
  vec4 pos = vec4(1.0f, 2.0f, 3.0f, 4.0f);
  tint_symbol.pos = pos;
}
VSOut main() {
  VSOut tint_symbol = VSOut(vec4(0.0f));
  foo(tint_symbol);
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:11: 'structure' :  entry point cannot return a value
ERROR: 0:11: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
