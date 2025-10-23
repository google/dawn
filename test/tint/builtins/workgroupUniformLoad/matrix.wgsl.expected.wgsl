var<workgroup> v : mat3x3<f32>;

fn foo() -> mat3x3<f32> {
  return workgroupUniformLoad(&(v));
}

@compute @workgroup_size(1)
fn main() {
  _ = foo();
}
