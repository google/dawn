@compute @workgroup_size(1)
fn f() {
  for(var must_not_collide : i32 = 0; ; ) {
    break;
  }
  var must_not_collide : i32;
}
