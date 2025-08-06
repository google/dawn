#version 460
#extension GL_EXT_fragment_shader_barycentric: require
precision highp float;
precision highp int;

layout(location = 0) out vec4 main_loc0_Output;
vec4 main_inner(vec3 bary_coord) {
  return vec4(bary_coord, 1.0f);
}
void main() {
  main_loc0_Output = main_inner(gl_BaryCoordEXT);
}
