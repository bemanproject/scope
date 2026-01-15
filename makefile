# Standard stuff

.SUFFIXES:

MAKEFLAGS+= --no-builtin-rules  # Disable the built-in implicit rules.
# MAKEFLAGS+= --warn-undefined-variables        # Warn when an undefined variable is referenced.
# MAKEFLAGS+= --include-dir=$(CURDIR)/conan     # Search DIRECTORY for included makefiles (*.mk).

export hostSystemName=$(shell uname)

ifeq (${hostSystemName},Darwin)
  export LLVM_PREFIX:=$(shell brew --prefix llvm)
  export LLVM_DIR:=$(shell realpath ${LLVM_PREFIX})
  export PATH:=${LLVM_DIR}/bin:${PATH}

  export CMAKE_CXX_STDLIB_MODULES_JSON=${LLVM_DIR}/lib/c++/libc++.modules.json
  export CXX=clang++
  export LDFLAGS=-L$(LLVM_DIR)/lib/c++ -lc++abi # XXX -lc++ -lc++experimental
  # FIXME: export GCOV="llvm-cov gcov"

  ### TODO: to test g++-15:
  export GCC_PREFIX:=$(shell brew --prefix gcc)
  export GCC_DIR:=$(shell realpath ${GCC_PREFIX})

  # export CMAKE_CXX_STDLIB_MODULES_JSON=${GCC_DIR}/lib/gcc/current/libstdc++.modules.json
  # export CXX:=g++-15
  # export CXXFLAGS:=-stdlib=libstdc++
  # export GCOV="gcov"
else ifeq (${hostSystemName},Linux)
  export LLVM_DIR=/usr/lib/llvm-20
  export PATH:=${LLVM_DIR}/bin:${PATH}
  export CXX=clang++-20
endif

.PHONY: all install coverage gclean distclean

all: build/compile_commands.json
	ln -sf $< .
	ninja -C build

build/compile_commands.json: CMakeLists.txt makefile
	cmake -S . -B build -G Ninja --log-level=DEBUG -D CMAKE_BUILD_TYPE=Release \
	 -D CMAKE_EXPERIMENTAL_CXX_IMPORT_STD="d0edc3af-4c50-42ea-a356-e2862fe7a444" \
	 -D CMAKE_CXX_STDLIB_MODULES_JSON=${CMAKE_CXX_STDLIB_MODULES_JSON} \
	 -D CMAKE_CXX_STANDARD=23 -D CMAKE_CXX_EXTENSIONS=YES -D CMAKE_CXX_STANDARD_REQUIRED=YES \
	 -D CMAKE_CXX_FLAGS='-fno-inline --coverage' \
	 -D CMAKE_CXX_MODULE_STD=NO \
	 -D CMAKE_INSTALL_MESSAGE=LAZY # XXX -D CMAKE_SKIP_INSTALL_RULES=YES # --fresh

install: build/cmake_install.cmake
	cmake --install build

distclean: clean
	rm -rf build
	find . -name '*~' -delete

gclean: clean
	find build -name '*.gc..' -delete

build/coverage: test
	mkdir -p $@

coverage: build/coverage
	gcovr  --merge-mode-functions separate

# Anything we don't know how to build will use this rule.
% ::
	ninja -C build $(@)
