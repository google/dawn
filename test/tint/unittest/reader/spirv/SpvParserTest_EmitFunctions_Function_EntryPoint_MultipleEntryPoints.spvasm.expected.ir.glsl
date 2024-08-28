SKIP: FAILED

#version 310 es

void first_shader_1() {
}
void main() {
  first_shader_1();
}
void main() {
  first_shader_1();
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'main' : function already has a body 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

void first_shader_1() {
}
void main() {
  first_shader_1();
}
void main() {
  first_shader_1();
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'main' : function already has a body 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
