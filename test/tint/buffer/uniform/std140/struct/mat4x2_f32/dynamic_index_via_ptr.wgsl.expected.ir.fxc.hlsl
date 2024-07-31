SKIP: FAILED


struct Inner {
  @size(64)
  m : mat4x2<f32>,
}

struct Outer {
  a : array<Inner, 4>,
}

@group(0) @binding(0) var<uniform> a : array<Outer, 4>;

var<private> counter = 0;

fn i() -> i32 {
  counter++;
  return counter;
}

@compute @workgroup_size(1)
fn f() {
  let p_a = &(a);
  let p_a_i = &((*(p_a))[i()]);
  let p_a_i_a = &((*(p_a_i)).a);
  let p_a_i_a_i = &((*(p_a_i_a))[i()]);
  let p_a_i_a_i_m = &((*(p_a_i_a_i)).m);
  let p_a_i_a_i_m_i = &((*(p_a_i_a_i_m))[i()]);
  let l_a : array<Outer, 4> = *(p_a);
  let l_a_i : Outer = *(p_a_i);
  let l_a_i_a : array<Inner, 4> = *(p_a_i_a);
  let l_a_i_a_i : Inner = *(p_a_i_a_i);
  let l_a_i_a_i_m : mat4x2<f32> = *(p_a_i_a_i_m);
  let l_a_i_a_i_m_i : vec2<f32> = *(p_a_i_a_i_m_i);
  let l_a_i_a_i_m_i_i : f32 = (*(p_a_i_a_i_m_i))[i()];
}

Failed to generate: :38:24 error: binary: %23 is not in scope
    %22:u32 = add %21, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:68:5 note: %23 declared here
    %23:u32 = mul %56, 4u
    ^^^^^^^

:43:24 error: binary: %23 is not in scope
    %29:u32 = add %28, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:68:5 note: %23 declared here
    %23:u32 = mul %56, 4u
    ^^^^^^^

:51:24 error: binary: %23 is not in scope
    %39:u32 = add %38, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:68:5 note: %23 declared here
    %23:u32 = mul %56, 4u
    ^^^^^^^

:68:5 error: binary: no matching overload for 'operator * (i32, u32)'

9 candidate operators:
 • 'operator * (T  ✓ , T  ✗ ) -> T' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (vecN<T>  ✗ , T  ✓ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✓ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✗ , matNxM<T>  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (matNxM<T>  ✗ , T  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (matCxR<T>  ✗ , vecC<T>  ✗ ) -> vecR<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecR<T>  ✗ , matCxR<T>  ✗ ) -> vecC<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (matKxR<T>  ✗ , matCxK<T>  ✗ ) -> matCxR<T>' where:
      ✗  'T' is 'f32' or 'f16'

    %23:u32 = mul %56, 4u
    ^^^^^^^^^^^^^^^^^^^^^

:24:3 note: in block
  $B3: {
  ^^^

note: # Disassembly
Inner = struct @align(8) {
  m:mat4x2<f32> @offset(0)
}

Outer = struct @align(8) {
  a:array<Inner, 4> @offset(0)
}

$B1: {  # root
  %a:ptr<uniform, array<vec4<u32>, 64>, read> = var @binding_point(0, 0)
  %counter:ptr<private, i32, read_write> = var, 0i
}

%i = func():i32 {
  $B2: {
    %4:i32 = load %counter
    %5:i32 = add %4, 1i
    store %counter, %5
    %6:i32 = load %counter
    ret %6
  }
}
%f = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %8:i32 = call %i
    %9:u32 = convert %8
    %10:u32 = mul 256u, %9
    %11:i32 = call %i
    %12:u32 = convert %11
    %13:u32 = mul 64u, %12
    %14:i32 = call %i
    %15:u32 = convert %14
    %16:u32 = mul 8u, %15
    %17:array<Outer, 4> = call %18, 0u
    %l_a:array<Outer, 4> = let %17
    %20:u32 = add %10, %13
    %21:u32 = add %20, %16
    %22:u32 = add %21, %23
    %24:Outer = call %25, %22
    %l_a_i:Outer = let %24
    %27:u32 = add %10, %13
    %28:u32 = add %27, %16
    %29:u32 = add %28, %23
    %30:array<Inner, 4> = call %31, %29
    %l_a_i_a:array<Inner, 4> = let %30
    %33:u32 = add %10, %13
    %34:Inner = call %35, %33
    %l_a_i_a_i:Inner = let %34
    %37:u32 = add %10, %13
    %38:u32 = add %37, %16
    %39:u32 = add %38, %23
    %40:mat4x2<f32> = call %41, %39
    %l_a_i_a_i_m:mat4x2<f32> = let %40
    %43:u32 = add %10, %13
    %44:u32 = add %43, %16
    %45:u32 = div %44, 16u
    %46:ptr<uniform, vec4<u32>, read> = access %a, %45
    %47:u32 = mod %44, 16u
    %48:u32 = div %47, 4u
    %49:vec4<u32> = load %46
    %50:vec2<u32> = swizzle %49, zw
    %51:vec2<u32> = swizzle %49, xy
    %52:bool = eq %48, 2u
    %53:vec2<u32> = hlsl.ternary %51, %50, %52
    %54:vec2<f32> = bitcast %53
    %l_a_i_a_i_m_i:vec2<f32> = let %54
    %56:i32 = call %i
    %23:u32 = mul %56, 4u
    %57:u32 = add %10, %13
    %58:u32 = add %57, %16
    %59:u32 = add %58, %23
    %60:u32 = div %59, 16u
    %61:ptr<uniform, vec4<u32>, read> = access %a, %60
    %62:u32 = mod %59, 16u
    %63:u32 = div %62, 4u
    %64:u32 = load_vector_element %61, %63
    %65:f32 = bitcast %64
    %l_a_i_a_i_m_i_i:f32 = let %65
    ret
  }
}
%35 = func(%start_byte_offset:u32):Inner {
  $B4: {
    %68:mat4x2<f32> = call %41, %start_byte_offset
    %69:Inner = construct %68
    ret %69
  }
}
%41 = func(%start_byte_offset_1:u32):mat4x2<f32> {  # %start_byte_offset_1: 'start_byte_offset'
  $B5: {
    %71:u32 = div %start_byte_offset_1, 16u
    %72:ptr<uniform, vec4<u32>, read> = access %a, %71
    %73:u32 = mod %start_byte_offset_1, 16u
    %74:u32 = div %73, 4u
    %75:vec4<u32> = load %72
    %76:vec2<u32> = swizzle %75, zw
    %77:vec2<u32> = swizzle %75, xy
    %78:bool = eq %74, 2u
    %79:vec2<u32> = hlsl.ternary %77, %76, %78
    %80:vec2<f32> = bitcast %79
    %81:u32 = add 8u, %start_byte_offset_1
    %82:u32 = div %81, 16u
    %83:ptr<uniform, vec4<u32>, read> = access %a, %82
    %84:u32 = mod %81, 16u
    %85:u32 = div %84, 4u
    %86:vec4<u32> = load %83
    %87:vec2<u32> = swizzle %86, zw
    %88:vec2<u32> = swizzle %86, xy
    %89:bool = eq %85, 2u
    %90:vec2<u32> = hlsl.ternary %88, %87, %89
    %91:vec2<f32> = bitcast %90
    %92:u32 = add 16u, %start_byte_offset_1
    %93:u32 = div %92, 16u
    %94:ptr<uniform, vec4<u32>, read> = access %a, %93
    %95:u32 = mod %92, 16u
    %96:u32 = div %95, 4u
    %97:vec4<u32> = load %94
    %98:vec2<u32> = swizzle %97, zw
    %99:vec2<u32> = swizzle %97, xy
    %100:bool = eq %96, 2u
    %101:vec2<u32> = hlsl.ternary %99, %98, %100
    %102:vec2<f32> = bitcast %101
    %103:u32 = add 24u, %start_byte_offset_1
    %104:u32 = div %103, 16u
    %105:ptr<uniform, vec4<u32>, read> = access %a, %104
    %106:u32 = mod %103, 16u
    %107:u32 = div %106, 4u
    %108:vec4<u32> = load %105
    %109:vec2<u32> = swizzle %108, zw
    %110:vec2<u32> = swizzle %108, xy
    %111:bool = eq %107, 2u
    %112:vec2<u32> = hlsl.ternary %110, %109, %111
    %113:vec2<f32> = bitcast %112
    %114:mat4x2<f32> = construct %80, %91, %102, %113
    ret %114
  }
}
%31 = func(%start_byte_offset_2:u32):array<Inner, 4> {  # %start_byte_offset_2: 'start_byte_offset'
  $B6: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat4x2<f32>(vec2<f32>(0.0f))))  # %a_1: 'a'
    loop [i: $B7, b: $B8, c: $B9] {  # loop_1
      $B7: {  # initializer
        next_iteration 0u  # -> $B8
      }
      $B8 (%idx:u32): {  # body
        %118:bool = gte %idx, 4u
        if %118 [t: $B10] {  # if_1
          $B10: {  # true
            exit_loop  # loop_1
          }
        }
        %119:u32 = mul %idx, 64u
        %120:u32 = add %start_byte_offset_2, %119
        %121:ptr<function, Inner, read_write> = access %a_1, %idx
        %122:Inner = call %35, %120
        store %121, %122
        continue  # -> $B9
      }
      $B9: {  # continuing
        %123:u32 = add %idx, 1u
        next_iteration %123  # -> $B8
      }
    }
    %124:array<Inner, 4> = load %a_1
    ret %124
  }
}
%25 = func(%start_byte_offset_3:u32):Outer {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %126:array<Inner, 4> = call %31, %start_byte_offset_3
    %127:Outer = construct %126
    ret %127
  }
}
%18 = func(%start_byte_offset_4:u32):array<Outer, 4> {  # %start_byte_offset_4: 'start_byte_offset'
  $B12: {
    %a_2:ptr<function, array<Outer, 4>, read_write> = var, array<Outer, 4>(Outer(array<Inner, 4>(Inner(mat4x2<f32>(vec2<f32>(0.0f))))))  # %a_2: 'a'
    loop [i: $B13, b: $B14, c: $B15] {  # loop_2
      $B13: {  # initializer
        next_iteration 0u  # -> $B14
      }
      $B14 (%idx_1:u32): {  # body
        %131:bool = gte %idx_1, 4u
        if %131 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %132:u32 = mul %idx_1, 256u
        %133:u32 = add %start_byte_offset_4, %132
        %134:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %135:Outer = call %25, %133
        store %134, %135
        continue  # -> $B15
      }
      $B15: {  # continuing
        %136:u32 = add %idx_1, 1u
        next_iteration %136  # -> $B14
      }
    }
    %137:array<Outer, 4> = load %a_2
    ret %137
  }
}

