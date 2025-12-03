struct Strct1 {
  Positions : array<vec4<f32>, 4u>,
}

@group(0u) @binding(0u) var<uniform> a : Strct1;

struct Strct2 {
  Colors : array<vec4<f32>, 4u>,
}

@group(0u) @binding(0u) var<uniform> b : Strct2;

@compute @workgroup_size(1u, 1u, 1u)
fn main() {
}
