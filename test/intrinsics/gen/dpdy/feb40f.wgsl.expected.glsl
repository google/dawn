SKIP: FAILED

#version 310 es
precision mediump float;

void dpdy_feb40f() {
  vec3 res = ddy(vec3(0.0f, 0.0f, 0.0f));
}

void fragment_main() {
  dpdy_feb40f();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:5: 'ddy' : no matching overloaded function found 
ERROR: 0:5: '=' :  cannot convert from ' const float' to ' temp mediump 3-component vector of float'
ERROR: 0:5: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



