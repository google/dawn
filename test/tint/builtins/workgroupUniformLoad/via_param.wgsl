var<workgroup> v : array<i32, 4>;

fn foo(p : ptr<workgroup, i32>) -> i32 {
  return workgroupUniformLoad(p);
}

fn bar() -> i32 {
  return foo(&(v[0]));
}

@compute @workgroup_size(1)
fn main() {
    _ = bar();
}
