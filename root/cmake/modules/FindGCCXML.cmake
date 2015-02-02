# - Try to find GCCXML
#
#  GCCXML_EXECUTABLE  - path to the GCCXML executable
#  GCCXML_FOUND       - True if GCCXML found.


find_program(GCCXML_EXECUTABLE NAMES gccxml
     PATHS ${GCCXML_DIR}/bin $ENV{GCCXML_DIR}/bin
)

if(GCCXML_EXECUTABLE)
  set(GCCXML_FOUND TRUE)
endif()

mark_as_advanced(
  GCCXML_EXECUTABLE
)

