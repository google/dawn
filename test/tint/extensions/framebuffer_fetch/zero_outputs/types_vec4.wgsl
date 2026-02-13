enable chromium_experimental_framebuffer_fetch;

@fragment fn f(@color(0) zero : vec4i, @color(1) one: vec4u, @color(2) two: vec4f) {
  g(zero, one, two);
}

fn g(g: vec4i, h: vec4u, i: vec4f) {}
