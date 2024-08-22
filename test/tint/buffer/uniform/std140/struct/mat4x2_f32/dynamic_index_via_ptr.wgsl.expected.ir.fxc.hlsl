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
%32 = func(%start_byte_offset:u32):mat4x2<f32> {
  $B4: {
    %60:u32 = div %start_byte_offset, 16u
    %61:ptr<uniform, vec4<u32>, read> = access %a, %60
    %62:u32 = mod %start_byte_offset, 16u
    %63:u32 = div %62, 4u
    %64:vec4<u32> = load %61
    %65:vec2<u32> = swizzle %64, zw
    %66:vec2<u32> = swizzle %64, xy
    %67:bool = eq %63, 2u
    %68:vec2<u32> = hlsl.ternary %66, %65, %67
    %69:vec2<f32> = bitcast %68
    %70:u32 = add 8u, %start_byte_offset
    %71:u32 = div %70, 16u
    %72:ptr<uniform, vec4<u32>, read> = access %a, %71
    %73:u32 = mod %70, 16u
    %74:u32 = div %73, 4u
    %75:vec4<u32> = load %72
    %76:vec2<u32> = swizzle %75, zw
    %77:vec2<u32> = swizzle %75, xy
    %78:bool = eq %74, 2u
    %79:vec2<u32> = hlsl.ternary %77, %76, %78
    %80:vec2<f32> = bitcast %79
    %81:u32 = add 16u, %start_byte_offset
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
    %92:u32 = add 24u, %start_byte_offset
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
    %103:mat4x2<f32> = construct %69, %80, %91, %102
    ret %103
  }
}
%28 = func(%start_byte_offset_1:u32):Inner {  # %start_byte_offset_1: 'start_byte_offset'
  $B5: {
    %105:mat4x2<f32> = call %32, %start_byte_offset_1
    %106:Inner = construct %105
    ret %106
  }
}
%24 = func(%start_byte_offset_2:u32):array<Inner, 4> {  # %start_byte_offset_2: 'start_byte_offset'
  $B6: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat4x2<f32>(vec2<f32>(0.0f))))  # %a_1: 'a'
    loop [i: $B7, b: $B8, c: $B9] {  # loop_1
      $B7: {  # initializer
        next_iteration 0u  # -> $B8
      }
      $B8 (%idx:u32): {  # body
        %110:bool = gte %idx, 4u
        if %110 [t: $B10] {  # if_1
          $B10: {  # true
            exit_loop  # loop_1
          }
        }
        %111:u32 = mul %idx, 64u
        %112:u32 = add %start_byte_offset_2, %111
        %113:ptr<function, Inner, read_write> = access %a_1, %idx
        %114:Inner = call %28, %112
        store %113, %114
        continue  # -> $B9
      }
      $B9: {  # continuing
        %115:u32 = add %idx, 1u
        next_iteration %115  # -> $B8
      }
    }
    %116:array<Inner, 4> = load %a_1
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

