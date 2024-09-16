SKIP: FAILED

#version 310 es


struct VertexInputs {
  uint vertex_index;
  uint instance_index;
};

vec4 tint_symbol_inner(VertexInputs inputs) {
  uint foo = (inputs.vertex_index + inputs.instance_index);
  return vec4(0.0f);
}
void main() {
  gl_Position = tint_symbol_inner(VertexInputs(gl_VertexID, gl_InstanceID));
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:14: 'constructor' :  cannot convert parameter 1 from ' gl_VertexId highp int VertexId' to ' global highp uint'
ERROR: 0:14: ' temp structure{ global highp uint vertex_index,  global highp uint instance_index}' : cannot construct with these arguments 
ERROR: 0:14: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
