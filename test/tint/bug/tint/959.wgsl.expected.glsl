#version 310 es
precision highp float;

struct S {
  float a;
  uint pad;
  uint pad_1;
  uint pad_2;
};

layout(binding = 0, std430) buffer b0_block_ssbo {
  S inner;
} b0;

layout(binding = 0, std430) buffer b0_block_ssbo_1 {
  S inner;
} b1;

layout(binding = 0, std430) buffer b0_block_ssbo_2 {
  S inner;
} b2;

layout(binding = 0, std430) buffer b0_block_ssbo_3 {
  S inner;
} b3;

layout(binding = 0, std430) buffer b0_block_ssbo_4 {
  S inner;
} b4;

layout(binding = 0, std430) buffer b0_block_ssbo_5 {
  S inner;
} b5;

layout(binding = 0, std430) buffer b0_block_ssbo_6 {
  S inner;
} b6;

layout(binding = 0, std430) buffer b0_block_ssbo_7 {
  S inner;
} b7;

layout(binding = 1, std140) uniform b0_block_ubo {
  S inner;
} b8;

layout(binding = 1, std140) uniform b0_block_ubo_1 {
  S inner;
} b9;

layout(binding = 1, std140) uniform b0_block_ubo_2 {
  S inner;
} b10;

layout(binding = 1, std140) uniform b0_block_ubo_3 {
  S inner;
} b11;

layout(binding = 1, std140) uniform b0_block_ubo_4 {
  S inner;
} b12;

layout(binding = 1, std140) uniform b0_block_ubo_5 {
  S inner;
} b13;

layout(binding = 1, std140) uniform b0_block_ubo_6 {
  S inner;
} b14;

layout(binding = 1, std140) uniform b0_block_ubo_7 {
  S inner;
} b15;

void tint_symbol() {
}

void main() {
  tint_symbol();
  return;
}
