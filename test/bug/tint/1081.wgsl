fn f(x : i32) -> i32 {
  if (x == 10) {
    discard;
  }
  return x;
}

[[stage(fragment)]]
fn main([[location(1)]] x: vec3<i32>) -> [[location(2)]] i32 {
  var y = x.x;
  loop {
    let r = f(y);
    if (r == 0) {
      break;
    }
  }
  return y;
}
