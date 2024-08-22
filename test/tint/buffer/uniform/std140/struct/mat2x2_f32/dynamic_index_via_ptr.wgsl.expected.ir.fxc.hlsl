SKIP: FAILED


struct Inner {
  @size(64)
  m : mat2x2<f32>,
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
  let l_a_i_a_i_m : mat2x2<f32> = *(p_a_i_a_i_m);
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
  m:mat2x2<f32> @offset(0)
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
    %31:mat2x2<f32> = call %32, %30
    %l_a_i_a_i_m:mat2x2<f32> = let %31
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
%18 = func(%start_byte_offset:u32):array<Outer, 4> {
  $B4: {
    %a_1:ptr<function, array<Outer, 4>, read_write> = var, array<Outer, 4>(Outer(array<Inner, 4>(Inner(mat2x2<f32>(vec2<f32>(0.0f))))))  # %a_1: 'a'
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
        %63:u32 = mul %idx, 256u
        %64:u32 = add %start_byte_offset, %63
        %65:ptr<function, Outer, read_write> = access %a_1, %idx
        %66:Outer = call %21, %64
        store %65, %66
        continue  # -> $B7
      }
      $B7: {  # continuing
        %67:u32 = add %idx, 1u
        next_iteration %67  # -> $B6
      }
    }
    %68:array<Outer, 4> = load %a_1
    ret %68
  }
}
%21 = func(%start_byte_offset_1:u32):Outer {  # %start_byte_offset_1: 'start_byte_offset'
  $B9: {
    %70:array<Inner, 4> = call %24, %start_byte_offset_1
    %71:Outer = construct %70
    ret %71
  }
}
%24 = func(%start_byte_offset_2:u32):array<Inner, 4> {  # %start_byte_offset_2: 'start_byte_offset'
  $B10: {
    %a_2:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat2x2<f32>(vec2<f32>(0.0f))))  # %a_2: 'a'
    loop [i: $B11, b: $B12, c: $B13] {  # loop_2
      $B11: {  # initializer
        next_iteration 0u  # -> $B12
      }
      $B12 (%idx_1:u32): {  # body
        %75:bool = gte %idx_1, 4u
        if %75 [t: $B14] {  # if_2
          $B14: {  # true
            exit_loop  # loop_2
          }
        }
        %76:u32 = mul %idx_1, 64u
        %77:u32 = add %start_byte_offset_2, %76
        %78:ptr<function, Inner, read_write> = access %a_2, %idx_1
        %79:Inner = call %28, %77
        store %78, %79
        continue  # -> $B13
      }
      $B13: {  # continuing
        %80:u32 = add %idx_1, 1u
        next_iteration %80  # -> $B12
      }
    }
    %81:array<Inner, 4> = load %a_2
    ret %81
  }
}
%28 = func(%start_byte_offset_3:u32):Inner {  # %start_byte_offset_3: 'start_byte_offset'
  $B15: {
    %83:mat2x2<f32> = call %32, %start_byte_offset_3
    %84:Inner = construct %83
    ret %84
  }
}
%32 = func(%start_byte_offset_4:u32):mat2x2<f32> {  # %start_byte_offset_4: 'start_byte_offset'
  $B16: {
    %86:u32 = div %start_byte_offset_4, 16u
    %87:ptr<uniform, vec4<u32>, read> = access %a, %86
    %88:u32 = mod %start_byte_offset_4, 16u
    %89:u32 = div %88, 4u
    %90:vec4<u32> = load %87
    %91:vec2<u32> = swizzle %90, zw
    %92:vec2<u32> = swizzle %90, xy
    %93:bool = eq %89, 2u
    %94:vec2<u32> = hlsl.ternary %92, %91, %93
    %95:vec2<f32> = bitcast %94
    %96:u32 = add 8u, %start_byte_offset_4
    %97:u32 = div %96, 16u
    %98:ptr<uniform, vec4<u32>, read> = access %a, %97
    %99:u32 = mod %96, 16u
    %100:u32 = div %99, 4u
    %101:vec4<u32> = load %98
    %102:vec2<u32> = swizzle %101, zw
    %103:vec2<u32> = swizzle %101, xy
    %104:bool = eq %100, 2u
    %105:vec2<u32> = hlsl.ternary %103, %102, %104
    %106:vec2<f32> = bitcast %105
    %107:mat2x2<f32> = construct %95, %106
    ret %107
  }
}

