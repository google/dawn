var<workgroup> v : bool;

fn foo() -> bool {
  return workgroupUniformLoad(&(v));
}

@compute @workgroup_size(1)
fn main() {
  _ = foo();
}
