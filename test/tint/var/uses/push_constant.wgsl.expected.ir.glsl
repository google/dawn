SKIP: FAILED

#version 310 es

uniform int a;
uniform int b;
uniform int c;
void uses_a() {
  int foo = a;
}
void uses_uses_a() {
  uses_a();
}
void uses_b() {
  int foo = b;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_a();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_uses_a();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_b();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
error: Error parsing GLSL shader:
ERROR: 0:20: 'main' : function already has a body 
ERROR: 0:20: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

uniform int a;
uniform int b;
uniform int c;
void uses_a() {
  int foo = a;
}
void uses_uses_a() {
  uses_a();
}
void uses_b() {
  int foo = b;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_a();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_uses_a();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_b();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
error: Error parsing GLSL shader:
ERROR: 0:20: 'main' : function already has a body 
ERROR: 0:20: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

uniform int a;
uniform int b;
uniform int c;
void uses_a() {
  int foo = a;
}
void uses_uses_a() {
  uses_a();
}
void uses_b() {
  int foo = b;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_a();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_uses_a();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_b();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
error: Error parsing GLSL shader:
ERROR: 0:20: 'main' : function already has a body 
ERROR: 0:20: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

uniform int a;
uniform int b;
uniform int c;
void uses_a() {
  int foo = a;
}
void uses_uses_a() {
  uses_a();
}
void uses_b() {
  int foo = b;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_a();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_uses_a();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_b();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
error: Error parsing GLSL shader:
ERROR: 0:20: 'main' : function already has a body 
ERROR: 0:20: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
