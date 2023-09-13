SKIP: FAILED


enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
  b : i32,
  c : f32,
}

var<pixel_local> P : PixelLocal;

fn f0() {
  P.a += 9;
}

fn f1() {
  f0();
  P.a += 8;
}

fn f2() {
  P.a += 7;
  f1();
}

@fragment
fn f() {
  f2();
}

Failed to generate: extensions/pixel_local/indirect_use/zero_outputs/multiple_attachments.wgsl:2:8 error: HLSL backend does not support extension 'chromium_experimental_pixel_local'
enable chromium_experimental_pixel_local;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

