SKIP: FAILED


@group(0) @binding(0) var t_f : texture_3d<f32>;

@group(0) @binding(1) var t_i : texture_3d<i32>;

@group(0) @binding(2) var t_u : texture_3d<u32>;

@compute @workgroup_size(1)
fn main() {
  var fdims = textureDimensions(t_f, 1);
  var idims = textureDimensions(t_i, 1);
  var udims = textureDimensions(t_u, 1);
}

Failed to generate: :19:51 error: var: initializer type 'vec2<u32>' does not match store type 'vec3<u32>'
    %fdims:ptr<function, vec3<u32>, read_write> = var, %14
                                                  ^^^

:8:3 note: in block
  $B2: {
  ^^^

:30:51 error: var: initializer type 'vec2<u32>' does not match store type 'vec3<u32>'
    %idims:ptr<function, vec3<u32>, read_write> = var, %25
                                                  ^^^

:8:3 note: in block
  $B2: {
  ^^^

:41:51 error: var: initializer type 'vec2<u32>' does not match store type 'vec3<u32>'
    %udims:ptr<function, vec3<u32>, read_write> = var, %36
                                                  ^^^

:8:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
$B1: {  # root
  %t_f:ptr<handle, texture_3d<f32>, read> = var @binding_point(0, 0)
  %t_i:ptr<handle, texture_3d<i32>, read> = var @binding_point(0, 1)
  %t_u:ptr<handle, texture_3d<u32>, read> = var @binding_point(0, 2)
}

%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %5:texture_3d<f32> = load %t_f
    %6:u32 = convert 1i
    %7:ptr<function, vec4<u32>, read_write> = var
    %8:ptr<function, u32, read_write> = access %7, 0u
    %9:ptr<function, u32, read_write> = access %7, 1u
    %10:ptr<function, u32, read_write> = access %7, 2u
    %11:ptr<function, u32, read_write> = access %7, 3u
    %12:void = %5.GetDimensions %6, %8, %9, %10, %11
    %13:vec4<u32> = load %7
    %14:vec2<u32> = swizzle %13, xyz
    %fdims:ptr<function, vec3<u32>, read_write> = var, %14
    %16:texture_3d<i32> = load %t_i
    %17:u32 = convert 1i
    %18:ptr<function, vec4<u32>, read_write> = var
    %19:ptr<function, u32, read_write> = access %18, 0u
    %20:ptr<function, u32, read_write> = access %18, 1u
    %21:ptr<function, u32, read_write> = access %18, 2u
    %22:ptr<function, u32, read_write> = access %18, 3u
    %23:void = %16.GetDimensions %17, %19, %20, %21, %22
    %24:vec4<u32> = load %18
    %25:vec2<u32> = swizzle %24, xyz
    %idims:ptr<function, vec3<u32>, read_write> = var, %25
    %27:texture_3d<u32> = load %t_u
    %28:u32 = convert 1i
    %29:ptr<function, vec4<u32>, read_write> = var
    %30:ptr<function, u32, read_write> = access %29, 0u
    %31:ptr<function, u32, read_write> = access %29, 1u
    %32:ptr<function, u32, read_write> = access %29, 2u
    %33:ptr<function, u32, read_write> = access %29, 3u
    %34:void = %27.GetDimensions %28, %30, %31, %32, %33
    %35:vec4<u32> = load %29
    %36:vec2<u32> = swizzle %35, xyz
    %udims:ptr<function, vec3<u32>, read_write> = var, %36
    ret
  }
}


tint executable returned error: exit status 1
