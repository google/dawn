// flags: --pixel_local_attachments 0=1
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

@fragment fn f() -> @location(0) vec4f {
  P.a += 42;
  return vec4f(2);
}
