struct a {
  a : i32,
}

@compute @workgroup_size(1)
fn f() {
  {
    let a : a = a();
    let b = a;
  }
  let a : a = a();
  let b = a;
}
