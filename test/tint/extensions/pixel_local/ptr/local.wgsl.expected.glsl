SKIP: INVALID


enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> V : PixelLocal;

@fragment
fn f() {
  let p = &(V);
  (*(p)).a = 42;
}

Failed to generate: <dawn>/test/tint/extensions/pixel_local/ptr/local.wgsl:2:8 error: GLSL backend does not support extension 'chromium_experimental_pixel_local'
enable chromium_experimental_pixel_local;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
