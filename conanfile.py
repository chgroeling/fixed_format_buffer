import os
import re

from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.files import copy


class FixedFormatBufferConan(ConanFile):
    name = "fixed_format_buffer"
    license = "MIT"
    author = "Christian Gröling <contact@christiangroeling.de>"
    url = "https://github.com/chgroeling/fixed_format_buffer"
    description = "Allocation-free fixed-capacity formatting buffer for embedded C++"
    topics = ("embedded", "formatting", "printf", "header-only")

    package_type = "header-library"
    settings = "os", "compiler", "build_type", "arch"

    exports_sources = "include/**"

    def set_version(self):
        header = os.path.join(self.recipe_folder, "include", "ffb", "fixed_format_buffer.h")
        text = open(header).read()
        major = re.search(r"#define FFB_VERSION_MAJOR (\d+)", text).group(1)
        minor = re.search(r"#define FFB_VERSION_MINOR (\d+)", text).group(1)
        patch = re.search(r"#define FFB_VERSION_PATCH (\d+)", text).group(1)
        self.version = f"{major}.{minor}.{patch}"

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        self.test_requires("gtest/1.17.0")

    def build_requirements(self):
        self.tool_requires("cmake/4.3.3")

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
