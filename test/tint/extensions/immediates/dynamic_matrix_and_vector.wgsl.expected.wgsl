var<immediate> pc : mat2x2f;

@group(0) @binding(0) var<storage, read_write> out : f32;

@group(0) @binding(1) var<uniform> indices : vec2u;

@compute @workgroup_size(1)
fn main() {
  out = pc[indices.x][indices.y];
}
