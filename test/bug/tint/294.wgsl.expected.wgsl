bug/tint/294.wgsl:8:24 warning: use of deprecated language feature: declare access with var<storage, read> instead of using [[access]] decoration
[[set(0), binding(1)]] var<storage> lights : [[access(read)]] Lights;
                       ^^^

struct Light {
  position : vec3<f32>;
  colour : vec3<f32>;
};

[[block]]
struct Lights {
  light : [[stride(32)]] array<Light>;
};

[[group(0), binding(1)]] var<storage, read> lights : Lights;
