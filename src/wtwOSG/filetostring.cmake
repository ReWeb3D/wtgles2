# This file is has been adapted from Wt
FILE(READ ${infile} f0)
STRING( LENGTH "${f0}" f0_LEN )
SET( f4 "// This is automatically generated code -- do not edit!\n// Generated from ${file} \nnamespace WTW\n{\n\n" )
SET( idx 0 )

# Max size for MSVC is 65K, but we're adding CRLFs here too.
SET( chunklength 55000)

WHILE( f0_LEN GREATER 0 )
  MATH( EXPR idx "${idx} + 1" )
  IF( f0_LEN GREATER ${chunklength} )
    STRING( SUBSTRING "${f0}" 0 ${chunklength} f3_CHUNK )
    STRING( REGEX REPLACE "\\\\" "\\\\\\\\" f1 "${f3_CHUNK}" )
    STRING( REGEX REPLACE "\"" "\\\\\"" f2 "${f1}" )
    STRING( REGEX REPLACE "\r?\n" "\\\\r\\\\n\"\n  \"" f3 "${f2}" )
    
    #SET( f4 "${f4}\n  static const std::string * ${var}${idx} = \"${f3}\";" )
	SET( f4 "${f4}\n  static const char * ${var} = \"${f3}\";" )
    MATH( EXPR f0_LEN "${f0_LEN} - ${chunklength}" )
    STRING( SUBSTRING "${f0}" ${chunklength} ${f0_LEN} f0)       
  ELSE(f0_LEN GREATER ${chunklength})
    STRING( SUBSTRING "${f0}" 0 ${f0_LEN} f3_CHUNK )
    STRING( REGEX REPLACE "\\\\" "\\\\\\\\" f1 "${f3_CHUNK}" )
    STRING( REGEX REPLACE "\"" "\\\\\"" f2 "${f1}" )
    STRING( REGEX REPLACE "\r?\n" "\\\\r\\\\n\"\n  \"" f3 "${f2}" )
    SET( f4 "${f4}\n  static const char * ${var} = \"${f3}\";" )
    SET( f0_LEN 0 )
  ENDIF(f0_LEN GREATER ${chunklength})
ENDWHILE( f0_LEN GREATER 0 )
SET( f4 "${f4}\n}")
FILE(WRITE ${outfile} "${f4}")

