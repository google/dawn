SKIP: FAILED

#version 310 es

vec4 tint_symbol_inner(uint b) {
  return vec4(float(b));
}
void main() {
  gl_Position = tint_symbol_inner(gl_InstanceID);
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:7: 'tint_symbol_inner' : no matching overloaded function found 
ERROR: 0:7: 'assign' :  cannot convert from ' const float' to ' gl_Position highp 4-component vector of float Position'
ERROR: 0:7: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
