# - Find FFTW
# Find the native FFTW includes and library
#
#  FFTW_INCLUDES    - where to find fftw3.h
#  FFTW_LIBRARIES   - List of libraries when using FFTW.
#  FFTW_FOUND       - True if FFTW found.

if (CEREAL_INCLUDES)
  # Already in cache, be silent
  set (CEREAL_FIND_QUIETLY TRUE)
endif (CEREAL_INCLUDES)

if( CEREAL_ROOT )
  find_path(CEREAL_INCLUDES
          NAMES cereal
          PATHS ${CEREAL_ROOT}
          PATH_SUFFIXES cereal
          )
elseif()
  find_path(CEREAL_INCLUDES
          NAMES cereal.hpp
          PATHS ${CMAKE_INSTALL_PREFIX}/include
          PATH_SUFFIXES cereal
          )
  find_path (CEREAL_INCLUDES Cereal)
endif()


# handle the QUIETLY and REQUIRED arguments and set FFTW_FOUND to TRUE if
# all listed variables are TRUE
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (CEREAL DEFAULT_MSG CEREAL_INCLUDES)

mark_as_advanced ( CEREAL_LIBRARIES CEREAL_INCLUDES)

message("Found Cereal: ${CEREAL_INCLUDES}")
