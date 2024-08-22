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

Failed to generate: :60:5 error: binary: no matching overload for 'operator * (i32, u32)'

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

    %48:u32 = mul %47, 4u
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
    %20:Outer = call %21, %10
    %l_a_i:Outer = let %20
    %23:array<Inner, 4> = call %24, %10
    %l_a_i_a:array<Inner, 4> = let %23
    %26:u32 = add %10, %13
    %27:Inner = call %28, %26
    %l_a_i_a_i:Inner = let %27
    %30:u32 = add %10, %13
    %31:mat4x2<f32> = call %32, %30
    %l_a_i_a_i_m:mat4x2<f32> = let %31
    %34:u32 = add %10, %13
    %35:u32 = add %34, %16
    %36:u32 = div %35, 16u
    %37:ptr<uniform, vec4<u32>, read> = access %a, %36
    %38:u32 = mod %35, 16u
    %39:u32 = div %38, 4u
    %40:vec4<u32> = load %37
    %41:vec2<u32> = swizzle %40, zw
    %42:vec2<u32> = swizzle %40, xy
    %43:bool = eq %39, 2u
    %44:vec2<u32> = hlsl.ternary %42, %41, %43
    %45:vec2<f32> = bitcast %44
    %l_a_i_a_i_m_i:vec2<f32> = let %45
    %47:i32 = call %i
    %48:u32 = mul %47, 4u
    %49:u32 = add %10, %13
    %50:u32 = add %49, %16
    %51:u32 = add %50, %48
    %52:u32 = div %51, 16u
    %53:ptr<uniform, vec4<u32>, read> = access %a, %52
    %54:u32 = mod %51, 16u
    %55:u32 = div %54, 4u
    %56:u32 = load_vector_element %53, %55
    %57:f32 = bitcast %56
    %l_a_i_a_i_m_i_i:f32 = let %57
    ret
  }
}
%24 = func(%start_byte_offset:u32):array<Inner, 4> {
  $B4: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat4x2<f32>(vec2<f32>(0.0f))))  # %a_1: 'a'
    loop [i: $B5, b: $B6, c: $B7] {  # loop_1
      $B5: {  # initializer
        next_iteration 0u  # -> $B6
      }
      $B6 (%idx:u32): {  # body
        %62:bool = gte %idx, 4u
        if %62 [t: $B8] {  # if_1
          $B8: {  # true
            exit_loop  # loop_1
          }
        }
        %63:u32 = mul %idx, 64u
        %64:u32 = add %start_byte_offset, %63
        %65:ptr<function, Inner, read_write> = access %a_1, %idx
        %66:Inner = call %28, %64
        store %65, %66
        continue  # -> $B7
      }
      $B7: {  # continuing
        %67:u32 = add %idx, 1u
        next_iteration %67  # -> $B6
      }
    }
    %68:array<Inner, 4> = load %a_1
    ret %68
  }
}
%28 = func(%start_byte_offset_1:u32):Inner {  # %start_byte_offset_1: 'start_byte_offset'
  $B9: {
    %70:mat4x2<f32> = call %32, %start_byte_offset_1
    %71:Inner = construct %70
    ret %71
  }
}
%32 = func(%start_byte_offset_2:u32):mat4x2<f32> {  # %start_byte_offset_2: 'start_byte_offset'
  $B10: {
    %73:u32 = div %start_byte_offset_2, 16u
    %74:ptr<uniform, vec4<u32>, read> = access %a, %73
    %75:u32 = mod %start_byte_offset_2, 16u
    %76:u32 = div %75, 4u
    %77:vec4<u32> = load %74
    %78:vec2<u32> = swizzle %77, zw
    %79:vec2<u32> = swizzle %77, xy
    %80:bool = eq %76, 2u
    %81:vec2<u32> = hlsl.ternary %79, %78, %80
    %82:vec2<f32> = bitcast %81
    %83:u32 = add 8u, %start_byte_offset_2
    %84:u32 = div %83, 16u
    %85:ptr<uniform, vec4<u32>, read> = access %a, %84
    %86:u32 = mod %83, 16u
    %87:u32 = div %86, 4u
    %88:vec4<u32> = load %85
    %89:vec2<u32> = swizzle %88, zw
    %90:vec2<u32> = swizzle %88, xy
    %91:bool = eq %87, 2u
    %92:vec2<u32> = hlsl.ternary %90, %89, %91
    %93:vec2<f32> = bitcast %92
    %94:u32 = add 16u, %start_byte_offset_2
    %95:u32 = div %94, 16u
    %96:ptr<uniform, vec4<u32>, read> = access %a, %95
    %97:u32 = mod %94, 16u
    %98:u32 = div %97, 4u
    %99:vec4<u32> = load %96
    %100:vec2<u32> = swizzle %99, zw
    %101:vec2<u32> = swizzle %99, xy
    %102:bool = eq %98, 2u
    %103:vec2<u32> = hlsl.ternary %101, %100, %102
    %104:vec2<f32> = bitcast %103
    %105:u32 = add 24u, %start_byte_offset_2
    %106:u32 = div %105, 16u
    %107:ptr<uniform, vec4<u32>, read> = access %a, %106
    %108:u32 = mod %105, 16u
    %109:u32 = div %108, 4u
    %110:vec4<u32> = load %107
    %111:vec2<u32> = swizzle %110, zw
    %112:vec2<u32> = swizzle %110, xy
    %113:bool = eq %109, 2u
    %114:vec2<u32> = hlsl.ternary %112, %111, %113
    %115:vec2<f32> = bitcast %114
    %116:mat4x2<f32> = construct %82, %93, %104, %115
    ret %116
  }
}
%21 = func(%start_byte_offset_3:u32):Outer {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %118:array<Inner, 4> = call %24, %start_byte_offset_3
    %119:Outer = construct %118
    ret %119
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
        %123:bool = gte %idx_1, 4u
        if %123 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %124:u32 = mul %idx_1, 256u
        %125:u32 = add %start_byte_offset_4, %124
        %126:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %127:Outer = call %21, %125
        store %126, %127
        continue  # -> $B15
      }
      $B15: {  # continuing
        %128:u32 = add %idx_1, 1u
        next_iteration %128  # -> $B14
      }
    }
    %129:array<Outer, 4> = load %a_2
    ret %129
  }
}

