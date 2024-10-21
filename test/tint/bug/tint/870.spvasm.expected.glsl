#version 310 es
precision highp float;
precision highp int;


struct sspp962805860buildInformationS {
  vec4 footprint;
  vec4 offset;
  int essence;
  int orientation[6];
  uint tint_pad_0;
};

struct x_B4_BuildInformation {
  sspp962805860buildInformationS passthru;
};

layout(binding = 2, std430)
buffer sspp962805860buildInformation_block_1_ssbo {
  x_B4_BuildInformation inner;
} v;
void main_1() {
  int orientation[6] = int[6](0, 0, 0, 0, 0, 0);
  int x_23[6] = v.inner.passthru.orientation;
  orientation[0] = x_23[0u];
  orientation[1] = x_23[1u];
  orientation[2] = x_23[2u];
  orientation[3] = x_23[3u];
  orientation[4] = x_23[4u];
  orientation[5] = x_23[5u];
}
void main() {
  main_1();
}
