#version 310 es
precision highp float;
precision highp int;


struct S {
  float a;
};

layout(binding = 0, std430)
buffer f_b0_block_ssbo {
  S inner;
} v;
layout(binding = 0, std430)
buffer f_b1_block_ssbo {
  S inner;
} v_1;
layout(binding = 0, std430)
buffer f_b2_block_ssbo {
  S inner;
} v_2;
layout(binding = 0, std430)
buffer f_b3_block_ssbo {
  S inner;
} v_3;
layout(binding = 0, std430)
buffer f_b4_block_ssbo {
  S inner;
} v_4;
layout(binding = 0, std430)
buffer f_b5_block_ssbo {
  S inner;
} v_5;
layout(binding = 0, std430)
buffer f_b6_block_ssbo {
  S inner;
} v_6;
layout(binding = 0, std430)
buffer f_b7_block_ssbo {
  S inner;
} v_7;
layout(binding = 1, std140)
uniform f_b8_block_ubo {
  S inner;
} v_8;
layout(binding = 1, std140)
uniform f_b9_block_ubo {
  S inner;
} v_9;
layout(binding = 1, std140)
uniform f_b10_block_ubo {
  S inner;
} v_10;
layout(binding = 1, std140)
uniform f_b11_block_ubo {
  S inner;
} v_11;
layout(binding = 1, std140)
uniform f_b12_block_ubo {
  S inner;
} v_12;
layout(binding = 1, std140)
uniform f_b13_block_ubo {
  S inner;
} v_13;
layout(binding = 1, std140)
uniform f_b14_block_ubo {
  S inner;
} v_14;
layout(binding = 1, std140)
uniform f_b15_block_ubo {
  S inner;
} v_15;
uniform highp sampler2D f_t0;
uniform highp sampler2D f_t1;
uniform highp sampler2D f_t2;
uniform highp sampler2D f_t3;
uniform highp sampler2D f_t4;
uniform highp sampler2D f_t5;
uniform highp sampler2D f_t6;
uniform highp sampler2D f_t7;
uniform highp sampler2DShadow f_t8;
uniform highp sampler2DShadow f_t9;
uniform highp sampler2DShadow f_t10;
uniform highp sampler2DShadow f_t11;
uniform highp sampler2DShadow f_t12;
uniform highp sampler2DShadow f_t13;
uniform highp sampler2DShadow f_t14;
uniform highp sampler2DShadow f_t15;
void main() {
}
