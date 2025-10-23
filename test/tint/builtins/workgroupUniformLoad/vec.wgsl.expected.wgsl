var<workgroup> v : vec4<f32>;

fn foo() -> vec4<f32> {
  return workgroupUniformLoad(&(v));
}

@compute @workgroup_size(1)
fn main() {
  _ = foo();
}
