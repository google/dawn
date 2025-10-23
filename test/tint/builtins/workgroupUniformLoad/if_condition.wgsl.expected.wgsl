var<workgroup> v : bool;

fn foo() -> i32 {
  if (workgroupUniformLoad(&(v))) {
    return 42;
  }
  return 0;
}

@compute @workgroup_size(1)
fn main() {
  _ = foo();
}
