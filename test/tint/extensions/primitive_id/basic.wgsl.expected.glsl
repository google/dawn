#version 310 es
#extension GL_EXT_geometry_shader: require
precision highp float;
precision highp int;

layout(location = 0) out vec4 main_loc0_Output;
vec4 main_inner(uint prim_id) {
  return vec4(float(prim_id));
}
void main() {
  main_loc0_Output = main_inner(uint(gl_PrimitiveID));
}
