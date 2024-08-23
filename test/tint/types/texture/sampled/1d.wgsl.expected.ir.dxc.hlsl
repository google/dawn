SKIP: FAILED


@group(0) @binding(0) var t_f : texture_1d<f32>;

@group(0) @binding(1) var t_i : texture_1d<i32>;

@group(0) @binding(2) var t_u : texture_1d<u32>;

@compute @workgroup_size(1)
fn main() {
  var fdims = textureDimensions(t_f, 1);
  var idims = textureDimensions(t_i, 1);
  var udims = textureDimensions(t_u, 1);
}

Failed to generate: :17:45 error: var: initializer type 'vec2<u32>' does not match store type 'u32'
    %fdims:ptr<function, u32, read_write> = var, %12
                                            ^^^

:8:3 note: in block
  $B2: {
  ^^^

:26:45 error: var: initializer type 'vec2<u32>' does not match store type 'u32'
    %idims:ptr<function, u32, read_write> = var, %21
                                            ^^^

:8:3 note: in block
  $B2: {
  ^^^

:35:45 error: var: initializer type 'vec2<u32>' does not match store type 'u32'
    %udims:ptr<function, u32, read_write> = var, %30
                                            ^^^

:8:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
$B1: {  # root
  %t_f:ptr<handle, texture_1d<f32>, read> = var @binding_point(0, 0)
  %t_i:ptr<handle, texture_1d<i32>, read> = var @binding_point(0, 1)
  %t_u:ptr<handle, texture_1d<u32>, read> = var @binding_point(0, 2)
}

%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %5:texture_1d<f32> = load %t_f
    %6:u32 = convert 1i
    %7:ptr<function, vec2<u32>, read_write> = var
    %8:ptr<function, u32, read_write> = access %7, 0u
    %9:ptr<function, u32, read_write> = access %7, 1u
    %10:void = %5.GetDimensions %6, %8, %9
    %11:vec2<u32> = load %7
    %12:vec2<u32> = swizzle %11, x
    %fdims:ptr<function, u32, read_write> = var, %12
    %14:texture_1d<i32> = load %t_i
    %15:u32 = convert 1i
    %16:ptr<function, vec2<u32>, read_write> = var
    %17:ptr<function, u32, read_write> = access %16, 0u
    %18:ptr<function, u32, read_write> = access %16, 1u
    %19:void = %14.GetDimensions %15, %17, %18
    %20:vec2<u32> = load %16
    %21:vec2<u32> = swizzle %20, x
    %idims:ptr<function, u32, read_write> = var, %21
    %23:texture_1d<u32> = load %t_u
    %24:u32 = convert 1i
    %25:ptr<function, vec2<u32>, read_write> = var
    %26:ptr<function, u32, read_write> = access %25, 0u
    %27:ptr<function, u32, read_write> = access %25, 1u
    %28:void = %23.GetDimensions %24, %26, %27
    %29:vec2<u32> = load %25
    %30:vec2<u32> = swizzle %29, x
    %udims:ptr<function, u32, read_write> = var, %30
    ret
  }
}


tint executable returned error: exit status 1
