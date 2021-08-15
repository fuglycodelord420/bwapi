include_directories(
  ../include
)

set(CMAKE_SHARED_LIBRARY_PREFIX "")

add_library(NullAIModule SHARED
  Source/Dll.cpp
  Source/NullAIModule.cpp
  )

target_link_libraries(NullAIModule BWAPILIB)
