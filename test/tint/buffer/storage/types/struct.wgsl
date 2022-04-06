struct Inner {
  f : f32,
};
struct S {
  inner : Inner,
};

@group(0) @binding(0)
var<storage, read> in : S;

@group(0) @binding(1)
var<storage, read_write> out : S;

@stage(compute) @workgroup_size(1)
fn main() {
  out = in;
}
