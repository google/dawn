SKIP: FAILED


enable chromium_experimental_framebuffer_fetch;

@fragment
fn f(@color(3) fbf : vec4f, @builtin(position) pos : vec4f) {
  g(fbf.w, pos.x);
}

fn g(a : f32, b : f32) {
}

Failed to generate: extensions/texel_fetch/additional_params/g.wgsl:1:8 error: HLSL backend does not support extension 'chromium_experimental_framebuffer_fetch'
enable chromium_experimental_framebuffer_fetch;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

