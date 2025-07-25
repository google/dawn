// flags: --pixel-local-attachments 0=1,1=6,2=3 --pixel-local-attachment-formats 0=R32Uint,1=R32Sint,2=R32Float
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
  b : i32,
  c : f32,
}

var<pixel_local> P : PixelLocal;

struct Out {
  @location(0) x : vec4f,
  @location(2) y : vec4f,
  @location(4) z : vec4f,
}

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

@fragment fn f() -> Out {
  f2();
  return Out(vec4f(10), vec4f(20), vec4f(30));
}
