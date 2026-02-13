enable chromium_experimental_framebuffer_fetch;

@fragment
fn f(@color(0) zero : vec3i, @color(1) one : vec3u, @color(2) two : vec3f) {
  g(zero, one, two);
}

fn g(g : vec3i, h : vec3u, i : vec3f) {
}
