SKIP: INVALID


enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
  b : i32,
  c : f32,
}

var<pixel_local> P : PixelLocal;

struct In {
  @invariant @builtin(position)
  pos : vec4f,
}

@fragment
fn f(tint_symbol : In) {
  P.a += u32(tint_symbol.pos.x);
}

Failed to generate: <dawn>/test/tint/extensions/pixel_local/entry_point_use/additional_params/invariant_builtin_in_struct.wgsl:2:8 error: GLSL backend does not support extension 'chromium_experimental_pixel_local'
enable chromium_experimental_pixel_local;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
