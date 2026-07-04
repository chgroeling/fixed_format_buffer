from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.files import copy


class FixedFormatBufferConan(ConanFile):
    name = "fixed_format_buffer"
    version = "0.1.0"
    license = "MIT"
    author = ""
    url = ""
    description = "Allocation-free fixed-capacity formatting buffer for embedded C++"
    topics = ("embedded", "formatting", "printf", "header-only")

    package_type = "header-library"
    settings = "os", "compiler", "build_type", "arch"

    exports_sources = "include/**"

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        self.test_requires("gtest/1.14.0")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()

    def package_id(self):
        self.info.clear()

    def package(self):
        copy(self, "*.h",
             src=self.source_folder,
             dst=self.package_folder)

    def package_info(self):
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.set_property("cmake_target_name", "fixed_format_buffer::fixed_format_buffer")
