// flags: --pixel_local_attachments 0=1 --pixel_local_attachment_formats 0=R32Uint
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> V : PixelLocal;

@fragment fn f() {
  let p = &V;
  (*p).a = 42;
}
