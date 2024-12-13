#version 310 es
precision highp float;
precision highp int;

layout(location = 2) in float tint_interstage_location2;
void tint_symbol_inner(float none) {
}
void main() {
  tint_symbol_inner(tint_interstage_location2);
}
