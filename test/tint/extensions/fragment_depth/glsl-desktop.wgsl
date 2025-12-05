// flags: --glsl-desktop

requires fragment_depth;

@fragment
fn any() -> @builtin(frag_depth, any) f32 { return 1.0; }

@fragment
fn less() -> @builtin(frag_depth, less) f32 { return 1.0; }

@fragment
fn greater() -> @builtin(frag_depth, greater) f32 { return 1.0; }
