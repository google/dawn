// flags: --pixel_local_attachments 0=1,1=6,2=3
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> V : PixelLocal;

@fragment fn f() {
  let p = &V;
  (*p).a = 42;
}
