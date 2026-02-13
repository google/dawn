enable chromium_experimental_framebuffer_fetch;

@fragment
fn f(@color(0) zero : vec2i, @color(1) one : vec2u, @color(2) two : vec2f) {
  g(zero, one, two);
}

fn g(d : vec2i, e : vec2u, f : vec2f) {
}
