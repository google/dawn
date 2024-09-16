SKIP: FAILED

#version 310 es

vec4 vs_main_inner(uint in_vertex_index) {
  return vec4[3](vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(1.0f, 1.0f, 0.0f, 1.0f))[in_vertex_index];
}
void main() {
  gl_Position = vs_main_inner(gl_VertexID);
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:7: 'vs_main_inner' : no matching overloaded function found 
ERROR: 0:7: 'assign' :  cannot convert from ' const float' to ' gl_Position highp 4-component vector of float Position'
ERROR: 0:7: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
