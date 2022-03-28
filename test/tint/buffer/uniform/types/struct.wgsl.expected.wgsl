struct Inner {
  f : f32,
}

struct S {
  inner : Inner,
}

@group(0) @binding(0) var<uniform> u : S;

@stage(compute) @workgroup_size(1)
fn main() {
  let x = u;
}
