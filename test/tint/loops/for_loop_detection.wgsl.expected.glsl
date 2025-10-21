#version 310 es

layout(binding = 0, std140)
uniform U_block_1_ubo {
  uvec4 inner[4];
} v;
layout(binding = 1, rgba32ui) uniform highp writeonly uimage2D dst_image2d;
shared uvec4 outputs[8][32];
uniform highp sampler2D src_image2d;
uvec4 tint_v4f32_to_v4u32(vec4 value) {
  return uvec4(clamp(value, vec4(0.0f), vec4(4294967040.0f)));
}
void main_inner(uvec3 lid, uint tint_local_index) {
  if ((tint_local_index < 256u)) {
    outputs[(tint_local_index / 32u)][(tint_local_index % 32u)] = uvec4(0u);
  }
  barrier();
  int init = int(lid.z);
  {
    int S = init;
    while(true) {
      int v_1 = S;
      uvec4 v_2 = v.inner[3u];
      if ((v_1 < int(v_2.x))) {
      } else {
        break;
      }
      {
        uint v_3 = uint(S);
        S = int((v_3 + uint(8)));
      }
      continue;
    }
  }
  {
    int s_group = 0;
    while(true) {
      int v_4 = s_group;
      uvec4 v_5 = v.inner[3u];
      if ((v_4 < int(v_5.z))) {
      } else {
        break;
      }
      uvec4 v_6 = v.inner[3u];
      outputs[lid.z][lid.x] = tint_v4f32_to_v4u32(texelFetch(src_image2d, ivec2(uvec2(uint(int(v_6.x)))), 0));
      barrier();
      uvec4 result = outputs[lid.z][lid.x];
      uvec4 v_7 = v.inner[3u];
      uvec2 v_8 = uvec2(uint(int(v_7.x)));
      uvec4 v_9 = result;
      imageStore(dst_image2d, ivec2(v_8), v_9);
      {
        uint v_10 = uint(s_group);
        s_group = int((v_10 + uint(8)));
      }
      continue;
    }
  }
}
layout(local_size_x = 32, local_size_y = 1, local_size_z = 8) in;
void main() {
  main_inner(gl_LocalInvocationID, gl_LocalInvocationIndex);
}
