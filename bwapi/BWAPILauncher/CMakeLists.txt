
include_directories(
  ../Util/Source
  ../BWAPI/openbw
  ../BWAPI/openbw/BWAPI
  ../BWAPI/Source/BWAPI
  ../BWAPI/Source
  ../BWAPICore
  ../Shared
)

add_executable(BWAPILauncher
  Source/Main.cpp
)

target_link_libraries(BWAPILauncher BWAPI "-lsimple_sdlcore -lsimple_graphical -lsimple_interactive -lSDL2")

if (NOT WIN32)
  target_link_libraries(BWAPILauncher dl "-lsimple_sdlcore" "-lsimple_graphical" "-lsimple_interactive" "-lSDL2")
endif()

install(TARGETS BWAPILauncher
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
