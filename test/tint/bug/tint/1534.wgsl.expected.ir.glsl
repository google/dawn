SKIP: FAILED

#version 310 es

struct g {
  uvec3 a;
};

struct h {
  uint a;
};

uniform g i;
h j;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint l = dot(i.a, i.a);
  j.a = i.a.x;
}
error: Error parsing GLSL shader:
ERROR: 0:15: 'dot' : no matching overloaded function found 
ERROR: 0:15: '=' :  cannot convert from ' const float' to ' temp highp uint'
ERROR: 0:15: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
