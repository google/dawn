#version 310 es


struct mat4x3_f32_std140 {
  vec3 col0;
  uint tint_pad_0;
  vec3 col1;
  uint tint_pad_1;
  vec3 col2;
  uint tint_pad_2;
  vec3 col3;
  uint tint_pad_3;
};

layout(binding = 0, std140)
uniform a_block_std140_1_ubo {
  mat4x3_f32_std140 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
} v_1;
int counter = 0;
int i() {
  uint v_2 = uint(counter);
  counter = int((v_2 + uint(1)));
  return counter;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_3 = min(uint(i()), 3u);
  mat4x3 v_4 = mat4x3(v.inner[v_3].col0, v.inner[v_3].col1, v.inner[v_3].col2, v.inner[v_3].col3);
  vec3 v_5 = v_4[min(uint(i()), 3u)];
  mat4x3_f32_std140 v_6[4] = v.inner;
  mat4x3 v_7[4] = mat4x3[4](mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)));
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      v_7[v_9] = mat4x3(v_6[v_9].col0, v_6[v_9].col1, v_6[v_9].col2, v_6[v_9].col3);
      {
        v_8 = (v_9 + 1u);
      }
      continue;
    }
  }
  mat4x3 l_a[4] = v_7;
  mat4x3 l_a_i = v_4;
  vec3 l_a_i_i = v_5;
  v_1.inner = (((v_5.x + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x);
}
