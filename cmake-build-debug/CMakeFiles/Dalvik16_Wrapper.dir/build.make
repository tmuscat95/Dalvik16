# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.9

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/clion-2017.3.3/bin/cmake/bin/cmake

# The command to remove a file.
RM = /opt/clion-2017.3.3/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/tim/Dropbox/Thesis/Dalvik16_Wrapper

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/tim/Dropbox/Thesis/Dalvik16_Wrapper/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/Dalvik16_Wrapper.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/Dalvik16_Wrapper.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Dalvik16_Wrapper.dir/flags.make

CMakeFiles/Dalvik16_Wrapper.dir/main.c.o: CMakeFiles/Dalvik16_Wrapper.dir/flags.make
CMakeFiles/Dalvik16_Wrapper.dir/main.c.o: ../main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/tim/Dropbox/Thesis/Dalvik16_Wrapper/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/Dalvik16_Wrapper.dir/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/Dalvik16_Wrapper.dir/main.c.o   -c /home/tim/Dropbox/Thesis/Dalvik16_Wrapper/main.c

CMakeFiles/Dalvik16_Wrapper.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/Dalvik16_Wrapper.dir/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/tim/Dropbox/Thesis/Dalvik16_Wrapper/main.c > CMakeFiles/Dalvik16_Wrapper.dir/main.c.i

CMakeFiles/Dalvik16_Wrapper.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/Dalvik16_Wrapper.dir/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/tim/Dropbox/Thesis/Dalvik16_Wrapper/main.c -o CMakeFiles/Dalvik16_Wrapper.dir/main.c.s

CMakeFiles/Dalvik16_Wrapper.dir/main.c.o.requires:

.PHONY : CMakeFiles/Dalvik16_Wrapper.dir/main.c.o.requires

CMakeFiles/Dalvik16_Wrapper.dir/main.c.o.provides: CMakeFiles/Dalvik16_Wrapper.dir/main.c.o.requires
	$(MAKE) -f CMakeFiles/Dalvik16_Wrapper.dir/build.make CMakeFiles/Dalvik16_Wrapper.dir/main.c.o.provides.build
.PHONY : CMakeFiles/Dalvik16_Wrapper.dir/main.c.o.provides

CMakeFiles/Dalvik16_Wrapper.dir/main.c.o.provides.build: CMakeFiles/Dalvik16_Wrapper.dir/main.c.o


CMakeFiles/Dalvik16_Wrapper.dir/interpreter.c.o: CMakeFiles/Dalvik16_Wrapper.dir/flags.make
CMakeFiles/Dalvik16_Wrapper.dir/interpreter.c.o: ../interpreter.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/tim/Dropbox/Thesis/Dalvik16_Wrapper/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/Dalvik16_Wrapper.dir/interpreter.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/Dalvik16_Wrapper.dir/interpreter.c.o   -c /home/tim/Dropbox/Thesis/Dalvik16_Wrapper/interpreter.c

CMakeFiles/Dalvik16_Wrapper.dir/interpreter.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/Dalvik16_Wrapper.dir/interpreter.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/tim/Dropbox/Thesis/Dalvik16_Wrapper/interpreter.c > CMakeFiles/Dalvik16_Wrapper.dir/interpreter.c.i

CMakeFiles/Dalvik16_Wrapper.dir/interpreter.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/Dalvik16_Wrapper.dir/interpreter.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/tim/Dropbox/Thesis/Dalvik16_Wrapper/interpreter.c -o CMakeFiles/Dalvik16_Wrapper.dir/interpreter.c.s

CMakeFiles/Dalvik16_Wrapper.dir/interpreter.c.o.requires:

.PHONY : CMakeFiles/Dalvik16_Wrapper.dir/interpreter.c.o.requires

CMakeFiles/Dalvik16_Wrapper.dir/interpreter.c.o.provides: CMakeFiles/Dalvik16_Wrapper.dir/interpreter.c.o.requires
	$(MAKE) -f CMakeFiles/Dalvik16_Wrapper.dir/build.make CMakeFiles/Dalvik16_Wrapper.dir/interpreter.c.o.provides.build
.PHONY : CMakeFiles/Dalvik16_Wrapper.dir/interpreter.c.o.provides

CMakeFiles/Dalvik16_Wrapper.dir/interpreter.c.o.provides.build: CMakeFiles/Dalvik16_Wrapper.dir/interpreter.c.o


CMakeFiles/Dalvik16_Wrapper.dir/loader.c.o: CMakeFiles/Dalvik16_Wrapper.dir/flags.make
CMakeFiles/Dalvik16_Wrapper.dir/loader.c.o: ../loader.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/tim/Dropbox/Thesis/Dalvik16_Wrapper/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/Dalvik16_Wrapper.dir/loader.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/Dalvik16_Wrapper.dir/loader.c.o   -c /home/tim/Dropbox/Thesis/Dalvik16_Wrapper/loader.c

CMakeFiles/Dalvik16_Wrapper.dir/loader.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/Dalvik16_Wrapper.dir/loader.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/tim/Dropbox/Thesis/Dalvik16_Wrapper/loader.c > CMakeFiles/Dalvik16_Wrapper.dir/loader.c.i

CMakeFiles/Dalvik16_Wrapper.dir/loader.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/Dalvik16_Wrapper.dir/loader.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/tim/Dropbox/Thesis/Dalvik16_Wrapper/loader.c -o CMakeFiles/Dalvik16_Wrapper.dir/loader.c.s

CMakeFiles/Dalvik16_Wrapper.dir/loader.c.o.requires:

.PHONY : CMakeFiles/Dalvik16_Wrapper.dir/loader.c.o.requires

CMakeFiles/Dalvik16_Wrapper.dir/loader.c.o.provides: CMakeFiles/Dalvik16_Wrapper.dir/loader.c.o.requires
	$(MAKE) -f CMakeFiles/Dalvik16_Wrapper.dir/build.make CMakeFiles/Dalvik16_Wrapper.dir/loader.c.o.provides.build
.PHONY : CMakeFiles/Dalvik16_Wrapper.dir/loader.c.o.provides

CMakeFiles/Dalvik16_Wrapper.dir/loader.c.o.provides.build: CMakeFiles/Dalvik16_Wrapper.dir/loader.c.o


# Object files for target Dalvik16_Wrapper
Dalvik16_Wrapper_OBJECTS = \
"CMakeFiles/Dalvik16_Wrapper.dir/main.c.o" \
"CMakeFiles/Dalvik16_Wrapper.dir/interpreter.c.o" \
"CMakeFiles/Dalvik16_Wrapper.dir/loader.c.o"

# External object files for target Dalvik16_Wrapper
Dalvik16_Wrapper_EXTERNAL_OBJECTS =

Dalvik16_Wrapper: CMakeFiles/Dalvik16_Wrapper.dir/main.c.o
Dalvik16_Wrapper: CMakeFiles/Dalvik16_Wrapper.dir/interpreter.c.o
Dalvik16_Wrapper: CMakeFiles/Dalvik16_Wrapper.dir/loader.c.o
Dalvik16_Wrapper: CMakeFiles/Dalvik16_Wrapper.dir/build.make
Dalvik16_Wrapper: CMakeFiles/Dalvik16_Wrapper.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/tim/Dropbox/Thesis/Dalvik16_Wrapper/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking C executable Dalvik16_Wrapper"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Dalvik16_Wrapper.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Dalvik16_Wrapper.dir/build: Dalvik16_Wrapper

.PHONY : CMakeFiles/Dalvik16_Wrapper.dir/build

CMakeFiles/Dalvik16_Wrapper.dir/requires: CMakeFiles/Dalvik16_Wrapper.dir/main.c.o.requires
CMakeFiles/Dalvik16_Wrapper.dir/requires: CMakeFiles/Dalvik16_Wrapper.dir/interpreter.c.o.requires
CMakeFiles/Dalvik16_Wrapper.dir/requires: CMakeFiles/Dalvik16_Wrapper.dir/loader.c.o.requires

.PHONY : CMakeFiles/Dalvik16_Wrapper.dir/requires

CMakeFiles/Dalvik16_Wrapper.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Dalvik16_Wrapper.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Dalvik16_Wrapper.dir/clean

CMakeFiles/Dalvik16_Wrapper.dir/depend:
	cd /home/tim/Dropbox/Thesis/Dalvik16_Wrapper/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/tim/Dropbox/Thesis/Dalvik16_Wrapper /home/tim/Dropbox/Thesis/Dalvik16_Wrapper /home/tim/Dropbox/Thesis/Dalvik16_Wrapper/cmake-build-debug /home/tim/Dropbox/Thesis/Dalvik16_Wrapper/cmake-build-debug /home/tim/Dropbox/Thesis/Dalvik16_Wrapper/cmake-build-debug/CMakeFiles/Dalvik16_Wrapper.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Dalvik16_Wrapper.dir/depend
