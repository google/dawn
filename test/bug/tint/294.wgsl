struct Light {
  position : vec3<f32>;
  colour : vec3<f32>;
};
[[block]] struct Lights {
  light : [[stride(32)]] array<Light>;
};
[[set(0), binding(1)]] var<storage> lights : [[access(read)]] Lights;
