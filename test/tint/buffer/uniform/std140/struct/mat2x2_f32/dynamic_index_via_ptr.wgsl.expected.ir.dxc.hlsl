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

Failed to generate: :38:24 error: binary: %23 is not in scope
    %22:u32 = add %21, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:68:5 note: %23 declared here
    %23:u32 = mul %56, 4u
    ^^^^^^^

:45:24 error: binary: %23 is not in scope
    %32:u32 = add %31, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:68:5 note: %23 declared here
    %23:u32 = mul %56, 4u
    ^^^^^^^

:50:24 error: binary: %23 is not in scope
    %38:u32 = add %37, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:68:5 note: %23 declared here
    %23:u32 = mul %56, 4u
    ^^^^^^^

:55:24 error: binary: %23 is not in scope
    %44:u32 = add %43, %23
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
    %20:u32 = add %10, %13
    %21:u32 = add %20, %16
    %22:u32 = add %21, %23
    %24:Outer = call %25, %22
    %l_a_i:Outer = let %24
    %27:array<Inner, 4> = call %28, %10
    %l_a_i_a:array<Inner, 4> = let %27
    %30:u32 = add %10, %13
    %31:u32 = add %30, %16
    %32:u32 = add %31, %23
    %33:Inner = call %34, %32
    %l_a_i_a_i:Inner = let %33
    %36:u32 = add %10, %13
    %37:u32 = add %36, %16
    %38:u32 = add %37, %23
    %39:mat2x2<f32> = call %40, %38
    %l_a_i_a_i_m:mat2x2<f32> = let %39
    %42:u32 = add %10, %13
    %43:u32 = add %42, %16
    %44:u32 = add %43, %23
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
%28 = func(%start_byte_offset:u32):array<Inner, 4> {
  $B4: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat2x2<f32>(vec2<f32>(0.0f))))  # %a_1: 'a'
    loop [i: $B5, b: $B6, c: $B7] {  # loop_1
      $B5: {  # initializer
        next_iteration 0u  # -> $B6
      }
      $B6 (%idx:u32): {  # body
        %70:bool = gte %idx, 4u
        if %70 [t: $B8] {  # if_1
          $B8: {  # true
            exit_loop  # loop_1
          }
        }
        %71:u32 = mul %idx, 64u
        %72:u32 = add %start_byte_offset, %71
        %73:ptr<function, Inner, read_write> = access %a_1, %idx
        %74:Inner = call %34, %72
        store %73, %74
        continue  # -> $B7
      }
      $B7: {  # continuing
        %75:u32 = add %idx, 1u
        next_iteration %75  # -> $B6
      }
    }
    %76:array<Inner, 4> = load %a_1
    ret %76
  }
}
%34 = func(%start_byte_offset_1:u32):Inner {  # %start_byte_offset_1: 'start_byte_offset'
  $B9: {
    %78:mat2x2<f32> = call %40, %start_byte_offset_1
    %79:Inner = construct %78
    ret %79
  }
}
%40 = func(%start_byte_offset_2:u32):mat2x2<f32> {  # %start_byte_offset_2: 'start_byte_offset'
  $B10: {
    %81:u32 = div %start_byte_offset_2, 16u
    %82:ptr<uniform, vec4<u32>, read> = access %a, %81
    %83:u32 = mod %start_byte_offset_2, 16u
    %84:u32 = div %83, 4u
    %85:vec4<u32> = load %82
    %86:vec2<u32> = swizzle %85, zw
    %87:vec2<u32> = swizzle %85, xy
    %88:bool = eq %84, 2u
    %89:vec2<u32> = hlsl.ternary %87, %86, %88
    %90:vec2<f32> = bitcast %89
    %91:u32 = add 8u, %start_byte_offset_2
    %92:u32 = div %91, 16u
    %93:ptr<uniform, vec4<u32>, read> = access %a, %92
    %94:u32 = mod %91, 16u
    %95:u32 = div %94, 4u
    %96:vec4<u32> = load %93
    %97:vec2<u32> = swizzle %96, zw
    %98:vec2<u32> = swizzle %96, xy
    %99:bool = eq %95, 2u
    %100:vec2<u32> = hlsl.ternary %98, %97, %99
    %101:vec2<f32> = bitcast %100
    %102:mat2x2<f32> = construct %90, %101
    ret %102
  }
}
%25 = func(%start_byte_offset_3:u32):Outer {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %104:array<Inner, 4> = call %28, %start_byte_offset_3
    %105:Outer = construct %104
    ret %105
  }
}
%18 = func(%start_byte_offset_4:u32):array<Outer, 4> {  # %start_byte_offset_4: 'start_byte_offset'
  $B12: {
    %a_2:ptr<function, array<Outer, 4>, read_write> = var, array<Outer, 4>(Outer(array<Inner, 4>(Inner(mat2x2<f32>(vec2<f32>(0.0f))))))  # %a_2: 'a'
    loop [i: $B13, b: $B14, c: $B15] {  # loop_2
      $B13: {  # initializer
        next_iteration 0u  # -> $B14
      }
      $B14 (%idx_1:u32): {  # body
        %109:bool = gte %idx_1, 4u
        if %109 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %110:u32 = mul %idx_1, 256u
        %111:u32 = add %start_byte_offset_4, %110
        %112:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %113:Outer = call %25, %111
        store %112, %113
        continue  # -> $B15
      }
      $B15: {  # continuing
        %114:u32 = add %idx_1, 1u
        next_iteration %114  # -> $B14
      }
    }
    %115:array<Outer, 4> = load %a_2
    ret %115
  }
}

