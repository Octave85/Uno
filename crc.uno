crc:
  104 101 97 100 0 0 0

  5 st
  4 !=
  while
  drop #
    st 8 <<
    4 dup
    st rot
    ^ :=

    6 st
    8 !=

    while
      drop
      4 st 32768 & dup

      1 >= if
        drop
        4 st 1 <<
        4129 ^
        4 swap :=
      end

      1 < if
        drop
        4 4 st 1 << :=
      end
      
      4 4 st 65535 & :=
    
      6 incat
      6 st 8 != #
    end

    6 0 :=

    5 incat
    5 st 4 !=
  end

end

(crc) #
 4 st out