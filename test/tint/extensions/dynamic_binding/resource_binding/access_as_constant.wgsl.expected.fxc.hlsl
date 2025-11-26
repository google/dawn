SKIP: FAILED

$B1: {  # root
  %sampled_textures:ptr<handle, resource_binding, read> = var undef @binding_point(0, 0)
}

%fs = @fragment func():void {
  $B2: {
    %3:resource_binding = load %sampled_textures
    %4:texture_1d<f32> = getBinding<texture_1d<f32>> %3, 2i
    %5:texture_1d<f32> = let %4
    %6:vec2<i32> = construct 0i, 0i
    %7:vec4<f32> = %5.Load %6
    %texture_load:vec4<f32> = let %7
    ret
  }
}
Failed to generate: :9:26 error: let: value type, 'texture_1d<f32>', must be concrete constructible type or a pointer type
    %5:texture_1d<f32> = let %4
                         ^^^

:6:3 note: in block
  $B2: {
  ^^^

:9:26 error: let: result type, 'texture_1d<f32>', must be concrete constructible type or a pointer type
    %5:texture_1d<f32> = let %4
                         ^^^

:6:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
$B1: {  # root
  %sampled_textures:ptr<handle, resource_binding, read> = var undef @binding_point(0, 0)
}

%fs = @fragment func():void {
  $B2: {
    %3:resource_binding = load %sampled_textures
    %4:texture_1d<f32> = getBinding<texture_1d<f32>> %3, 2i
    %5:texture_1d<f32> = let %4
    %6:vec2<i32> = construct 0i, 0i
    %7:vec4<f32> = %5.Load %6
    %texture_load:vec4<f32> = let %7
    ret
  }
}


tint executable returned error: exit status 1
