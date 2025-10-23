struct Light {
  position : vec3<f32>,
  colour : vec3<f32>,
}

struct Lights {
  light : array<Light>,
}

@group(0) @binding(1) var<storage, read> lights : Lights;

@compute @workgroup_size(1)
fn main() {
  _ = lights.light[0];
}
