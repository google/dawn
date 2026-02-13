enable chromium_experimental_framebuffer_fetch;

@fragment
fn f(@color(0) zero : i32, @color(1) one : u32, @color(2) two : f32) {
  g(zero, one, two);
}

fn g(a : i32, b : u32, c : f32) {
}
