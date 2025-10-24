alias a = i32;

fn f() {
  {
    const a : a = a();
    const b = a;
  }
  const a : a = a();
  const b = a;
}

@compute @workgroup_size(1)
fn main() {
  f();
}
