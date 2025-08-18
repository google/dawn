struct S {
  a : vec4<f32>,
  b : i32,
}

struct sb_block {
  inner : array<S>,
}

@group(0u) @binding(0u) var<storage, read> sb : sb_block;

@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  _ = sb.inner[1i];
}
