SKIP: FAILED


enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
  b : i32,
  c : f32,
}

var<pixel_local> P : PixelLocal;

struct In {
  @builtin(position)
  pos : vec4f,
}

@fragment
fn f(tint_symbol : In, @location(0) uv : vec4f) {
  P.a += (u32(tint_symbol.pos.x) + u32(uv.x));
}

Failed to generate: extensions/pixel_local/entry_point_use/additional_params/builtin_in_struct_and_location.wgsl:2:8 error: HLSL backend does not support extension 'chromium_experimental_pixel_local'
enable chromium_experimental_pixel_local;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

