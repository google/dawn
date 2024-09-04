SKIP: FAILED

#version 310 es

struct Interface {
  float col1;
  float col2;
  vec4 pos;
};
precision highp float;
precision highp int;


Interface main() {
  return Interface(0.40000000596046447754f, 0.60000002384185791016f, vec4(0.0f));
}
void main(Interface colors) {
  float r = colors.col1;
  float g = colors.col2;
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'structure' :  entry point cannot return a value
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

struct Interface {
  float col1;
  float col2;
  vec4 pos;
};
precision highp float;
precision highp int;


Interface main() {
  return Interface(0.40000000596046447754f, 0.60000002384185791016f, vec4(0.0f));
}
void main(Interface colors) {
  float r = colors.col1;
  float g = colors.col2;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
