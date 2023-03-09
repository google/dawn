#version 310 es
precision highp float;

struct sspp962805860buildInformationS {
  vec4 footprint;
  vec4 offset;
  int essence;
  int orientation[6];
  uint pad;
};

struct x_B4_BuildInformation {
  sspp962805860buildInformationS passthru;
};

layout(binding = 2, std430) buffer sspp962805860buildInformation_block_ssbo {
  x_B4_BuildInformation inner;
} sspp962805860buildInformation;

void main_1() {
  int orientation[6] = int[6](0, 0, 0, 0, 0, 0);
  int x_23[6] = sspp962805860buildInformation.inner.passthru.orientation;
  orientation[0] = x_23[0u];
  orientation[1] = x_23[1u];
  orientation[2] = x_23[2u];
  orientation[3] = x_23[3u];
  orientation[4] = x_23[4u];
  orientation[5] = x_23[5u];
  return;
}

void tint_symbol() {
  main_1();
}

void main() {
  tint_symbol();
  return;
}
