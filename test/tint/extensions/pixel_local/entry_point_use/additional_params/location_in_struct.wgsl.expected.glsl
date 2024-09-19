SKIP: INVALID


enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
  b : i32,
  c : f32,
}

var<pixel_local> P : PixelLocal;

struct In {
  @location(0)
  a : vec4f,
  @interpolate(flat) @location(1)
  b : vec4f,
}

@fragment
fn f(tint_symbol : In) {
  P.a += (u32(tint_symbol.a.x) + u32(tint_symbol.b.y));
}

Failed to generate: <dawn>/test/tint/extensions/pixel_local/entry_point_use/additional_params/location_in_struct.wgsl:2:8 error: GLSL backend does not support extension 'chromium_experimental_pixel_local'
enable chromium_experimental_pixel_local;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
