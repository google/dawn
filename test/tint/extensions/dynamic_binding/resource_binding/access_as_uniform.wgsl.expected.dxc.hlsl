SKIP: FAILED

$B1: {  # root
  %index:ptr<uniform, array<vec4<u32>, 1>, read> = var undef @binding_point(1, 0)
  %sampled_textures:ptr<handle, resource_binding, read> = var undef @binding_point(0, 0)
}

%fs = @fragment func():void {
  $B2: {
    %4:resource_binding = load %sampled_textures
    %5:ptr<uniform, vec4<u32>, read> = access %index, 0u
    %6:u32 = load_vector_element %5, 0u
    %7:texture_3d<f32> = getBinding<texture_3d<f32>> %4, %6
    %8:texture_3d<f32> = let %7
    %9:vec4<i32> = construct vec3<i32>(0i), 0i
    %10:vec4<f32> = %8.Load %9
    %texture_load:vec4<f32> = let %10
    ret
  }
}
Failed to generate: :12:26 error: let: value type, 'texture_3d<f32>', must be concrete constructible type or a pointer type
    %8:texture_3d<f32> = let %7
                         ^^^

:7:3 note: in block
  $B2: {
  ^^^

:12:26 error: let: result type, 'texture_3d<f32>', must be concrete constructible type or a pointer type
    %8:texture_3d<f32> = let %7
                         ^^^

:7:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
$B1: {  # root
  %index:ptr<uniform, array<vec4<u32>, 1>, read> = var undef @binding_point(1, 0)
  %sampled_textures:ptr<handle, resource_binding, read> = var undef @binding_point(0, 0)
}

%fs = @fragment func():void {
  $B2: {
    %4:resource_binding = load %sampled_textures
    %5:ptr<uniform, vec4<u32>, read> = access %index, 0u
    %6:u32 = load_vector_element %5, 0u
    %7:texture_3d<f32> = getBinding<texture_3d<f32>> %4, %6
    %8:texture_3d<f32> = let %7
    %9:vec4<i32> = construct vec3<i32>(0i), 0i
    %10:vec4<f32> = %8.Load %9
    %texture_load:vec4<f32> = let %10
    ret
  }
}


tint executable returned error: exit status 1
