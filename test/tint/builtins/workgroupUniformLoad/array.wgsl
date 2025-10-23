var<workgroup> v : array<i32, 4>;

fn foo() -> array<i32, 4> {
  return workgroupUniformLoad(&v);
}

@compute @workgroup_size(1)
fn main() {
    _ = foo();
}
