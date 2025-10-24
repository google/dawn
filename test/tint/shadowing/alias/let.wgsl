alias a = i32;

fn f() {
  {
    let a : a = a();
    let b = a;
  }
  let a : a = a();
  let b = a;
}

@compute @workgroup_size(1)
fn main() {
    f();
}
