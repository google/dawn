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
    %t1d:vec4<f32> = let %7
    %9:resource_binding = load %sampled_textures
    %10:texture_2d<i32> = getBinding<texture_2d<i32>> %9, 1i
    %11:texture_2d<i32> = let %10
    %12:vec3<i32> = construct vec2<i32>(0i, 1i), 0i
    %13:vec4<i32> = %11.Load %12
    %t2d:vec4<i32> = let %13
    %15:resource_binding = load %sampled_textures
    %16:texture_3d<u32> = getBinding<texture_3d<u32>> %15, 1i
    %17:texture_3d<u32> = let %16
    %18:vec4<i32> = construct vec3<i32>(2i, 1i, 0i), 0i
    %19:vec4<u32> = %17.Load %18
    %tcube:vec4<u32> = let %19
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

:15:27 error: let: value type, 'texture_2d<i32>', must be concrete constructible type or a pointer type
    %11:texture_2d<i32> = let %10
                          ^^^

:6:3 note: in block
  $B2: {
  ^^^

:15:27 error: let: result type, 'texture_2d<i32>', must be concrete constructible type or a pointer type
    %11:texture_2d<i32> = let %10
                          ^^^

:6:3 note: in block
  $B2: {
  ^^^

:21:27 error: let: value type, 'texture_3d<u32>', must be concrete constructible type or a pointer type
    %17:texture_3d<u32> = let %16
                          ^^^

:6:3 note: in block
  $B2: {
  ^^^

:21:27 error: let: result type, 'texture_3d<u32>', must be concrete constructible type or a pointer type
    %17:texture_3d<u32> = let %16
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
    %t1d:vec4<f32> = let %7
    %9:resource_binding = load %sampled_textures
    %10:texture_2d<i32> = getBinding<texture_2d<i32>> %9, 1i
    %11:texture_2d<i32> = let %10
    %12:vec3<i32> = construct vec2<i32>(0i, 1i), 0i
    %13:vec4<i32> = %11.Load %12
    %t2d:vec4<i32> = let %13
    %15:resource_binding = load %sampled_textures
    %16:texture_3d<u32> = getBinding<texture_3d<u32>> %15, 1i
    %17:texture_3d<u32> = let %16
    %18:vec4<i32> = construct vec3<i32>(2i, 1i, 0i), 0i
    %19:vec4<u32> = %17.Load %18
    %tcube:vec4<u32> = let %19
    ret
  }
}


tint executable returned error: exit status 1
