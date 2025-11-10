//
// any
//
#version 310 es
precision highp float;
precision highp int;


struct tint_immediate_struct {
  float tint_frag_depth_min;
  float tint_frag_depth_max;
};

layout(location = 0) uniform tint_immediate_struct tint_immediates;
float any_inner() {
  return 1.0f;
}
void main() {
  float v = any_inner();
  gl_FragDepth = clamp(v, tint_immediates.tint_frag_depth_min, tint_immediates.tint_frag_depth_max);
}
//
// less
//
#version 310 es
precision highp float;
precision highp int;


struct tint_immediate_struct {
  float tint_frag_depth_min;
  float tint_frag_depth_max;
};

layout(location = 0) uniform tint_immediate_struct tint_immediates;
float less_inner() {
  return 1.0f;
}
void main() {
  float v = less_inner();
  gl_FragDepth = clamp(v, tint_immediates.tint_frag_depth_min, tint_immediates.tint_frag_depth_max);
}
//
// greater
//
#version 310 es
precision highp float;
precision highp int;


struct tint_immediate_struct {
  float tint_frag_depth_min;
  float tint_frag_depth_max;
};

layout(location = 0) uniform tint_immediate_struct tint_immediates;
float greater_inner() {
  return 1.0f;
}
void main() {
  float v = greater_inner();
  gl_FragDepth = clamp(v, tint_immediates.tint_frag_depth_min, tint_immediates.tint_frag_depth_max);
}
