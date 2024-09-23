SKIP: FAILED

#version 310 es

uniform highp samplerCubeArray t_f_1;
uniform highp isamplerCubeArray t_i_1;
uniform highp usamplerCubeArray t_u_1;
void tint_symbol() {
  uvec2 fdims = uvec2(textureSize(t_f_1, 1).xy);
  uvec2 idims = uvec2(textureSize(t_i_1, 1).xy);
  uvec2 udims = uvec2(textureSize(t_u_1, 1).xy);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:3: 'samplerCubeArray' : Reserved word. 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
