var<immediate> pc : vec2f;
@group(0) @binding(0) var<storage, read_write> out : f32;
@group(0) @binding(1) var<uniform> idx : u32;

@compute @workgroup_size(1)
fn main() {
  out = pc[idx];
}

