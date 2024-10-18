SKIP: FAILED


enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
  b : i32,
  c : f32,
}

var<pixel_local> P : PixelLocal;

@fragment
fn f(@builtin(position) pos : vec4f) {
  P.a += u32(pos.x);
}

@fragment
fn f2(@builtin(position) pos : vec4f) {
  P.b += i32(pos.x);
}

@fragment
fn f3(@builtin(position) pos : vec4f) {
  P.c += pos.x;
}

Failed to generate: C:\src\dawn\test\tint\extensions\pixel_local\entry_point_use\additional_params\builtin_multiple_entry_points.wgsl:2:8 error: GLSL backend does not support extension 'chromium_experimental_pixel_local'
enable chromium_experimental_pixel_local;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
  b : i32,
  c : f32,
}

var<pixel_local> P : PixelLocal;

@fragment
fn f(@builtin(position) pos : vec4f) {
  P.a += u32(pos.x);
}

@fragment
fn f2(@builtin(position) pos : vec4f) {
  P.b += i32(pos.x);
}

@fragment
fn f3(@builtin(position) pos : vec4f) {
  P.c += pos.x;
}

Failed to generate: C:\src\dawn\test\tint\extensions\pixel_local\entry_point_use\additional_params\builtin_multiple_entry_points.wgsl:2:8 error: GLSL backend does not support extension 'chromium_experimental_pixel_local'
enable chromium_experimental_pixel_local;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
  b : i32,
  c : f32,
}

var<pixel_local> P : PixelLocal;

@fragment
fn f(@builtin(position) pos : vec4f) {
  P.a += u32(pos.x);
}

@fragment
fn f2(@builtin(position) pos : vec4f) {
  P.b += i32(pos.x);
}

@fragment
fn f3(@builtin(position) pos : vec4f) {
  P.c += pos.x;
}

Failed to generate: C:\src\dawn\test\tint\extensions\pixel_local\entry_point_use\additional_params\builtin_multiple_entry_points.wgsl:2:8 error: GLSL backend does not support extension 'chromium_experimental_pixel_local'
enable chromium_experimental_pixel_local;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
