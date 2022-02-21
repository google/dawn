SKIP: FAILED

builtins/gen/isFinite/426f9f.wgsl:28:19 warning: use of deprecated builtin
  var res: bool = isFinite(1.0);
                  ^^^^^^^^

#version 310 es

void isFinite_426f9f() {
  bool res = isfinite(1.0f);
}

vec4 vertex_main() {
  isFinite_426f9f();
  return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

void main() {
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
Error parsing GLSL shader:
ERROR: 0:4: 'isfinite' : no matching overloaded function found 
ERROR: 0:4: '=' :  cannot convert from ' const float' to ' temp bool'
ERROR: 0:4: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
precision mediump float;

void isFinite_426f9f() {
  bool res = isfinite(1.0f);
}

void fragment_main() {
  isFinite_426f9f();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:5: 'isfinite' : no matching overloaded function found 
ERROR: 0:5: '=' :  cannot convert from ' const float' to ' temp bool'
ERROR: 0:5: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es

void isFinite_426f9f() {
  bool res = isfinite(1.0f);
}

void compute_main() {
  isFinite_426f9f();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:4: 'isfinite' : no matching overloaded function found 
ERROR: 0:4: '=' :  cannot convert from ' const float' to ' temp bool'
ERROR: 0:4: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



