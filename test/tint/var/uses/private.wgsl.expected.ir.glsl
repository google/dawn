SKIP: FAILED

#version 310 es

int a = 0;
int b = 0;
int c = 0;
void uses_a() {
  a = (a + 1);
}
void uses_b() {
  b = (b * 2);
}
void uses_a_and_b() {
  b = a;
}
void no_uses() {
}
void outer() {
  a = 0;
  uses_a();
  uses_a_and_b();
  uses_b();
  no_uses();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a = 42;
  uses_a();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  b = 7;
  uses_b();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  outer();
  no_uses();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  no_uses();
}
error: Error parsing GLSL shader:
ERROR: 0:30: 'main' : function already has a body 
ERROR: 0:30: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

int a = 0;
int b = 0;
int c = 0;
void uses_a() {
  a = (a + 1);
}
void uses_b() {
  b = (b * 2);
}
void uses_a_and_b() {
  b = a;
}
void no_uses() {
}
void outer() {
  a = 0;
  uses_a();
  uses_a_and_b();
  uses_b();
  no_uses();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a = 42;
  uses_a();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  b = 7;
  uses_b();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  outer();
  no_uses();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  no_uses();
}
error: Error parsing GLSL shader:
ERROR: 0:30: 'main' : function already has a body 
ERROR: 0:30: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

int a = 0;
int b = 0;
int c = 0;
void uses_a() {
  a = (a + 1);
}
void uses_b() {
  b = (b * 2);
}
void uses_a_and_b() {
  b = a;
}
void no_uses() {
}
void outer() {
  a = 0;
  uses_a();
  uses_a_and_b();
  uses_b();
  no_uses();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a = 42;
  uses_a();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  b = 7;
  uses_b();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  outer();
  no_uses();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  no_uses();
}
error: Error parsing GLSL shader:
ERROR: 0:30: 'main' : function already has a body 
ERROR: 0:30: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

int a = 0;
int b = 0;
int c = 0;
void uses_a() {
  a = (a + 1);
}
void uses_b() {
  b = (b * 2);
}
void uses_a_and_b() {
  b = a;
}
void no_uses() {
}
void outer() {
  a = 0;
  uses_a();
  uses_a_and_b();
  uses_b();
  no_uses();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a = 42;
  uses_a();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  b = 7;
  uses_b();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  outer();
  no_uses();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  no_uses();
}
error: Error parsing GLSL shader:
ERROR: 0:30: 'main' : function already has a body 
ERROR: 0:30: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
