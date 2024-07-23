set_project("ca")
set_version("v0.1")

set_license("MIT")

add_rules("plugin.compile_commands.autoupdate", { outputdir = "$(buildir)" })

set_languages("c++20")

add_rules("mode.debug", "mode.release",
          "mode.coverage",
          "mode.asan", "mode.ubsan")

add_requires("catch2 3.x", "hedley 15", { system = false })

target("test")
    set_default(true)
    set_kind("binary")
    add_tests("ca")

    add_packages("catch2", "hedley")

    add_includedirs("include", "tests")
    add_files("tests/*.cpp")