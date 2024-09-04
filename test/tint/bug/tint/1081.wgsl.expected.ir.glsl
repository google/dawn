SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


bool continue_execution = true;
int f(int x) {
  if ((x == 10)) {
    continue_execution = false;
  }
  return x;
}
int main(ivec3 x) {
  int y = x[0u];
  {
    while(true) {
      int r = f(y);
      if ((r == 0)) {
        break;
      }
      {
      }
      continue;
    }
  }
  if (!(continue_execution)) {
    discard;
  }
  return y;
}
error: Error parsing GLSL shader:
ERROR: 0:13: 'main' : function cannot take any parameter(s) 
ERROR: 0:13: 'int' :  entry point cannot return a value
ERROR: 0:13: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
