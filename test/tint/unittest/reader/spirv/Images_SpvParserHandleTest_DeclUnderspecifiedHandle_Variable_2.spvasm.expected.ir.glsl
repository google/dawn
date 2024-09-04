SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


uniform highp writeonly image1D x_10;
void main_1() {
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:6: 'image1D' : Reserved word. 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
