SKIP: FAILED

#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float a = 1.0f;
  float b = 0.0f;
  float r = (a % b);
}
error: Error parsing GLSL shader:
ERROR: 0:7: '%' :  wrong operand types: no operation '%' exists that takes a left-hand operand of type ' temp highp float' and a right operand of type ' temp highp float' (or there is no acceptable conversion)
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
