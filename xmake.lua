set_project("coal")
set_version("v0.1")

set_license("MIT")

add_rules("plugin.compile_commands.autoupdate", { outputdir = "$(buildir)" })

set_languages("c++20")

add_rules("mode.debug", "mode.release",
          "mode.coverage")

add_requires("catch2 3.x")

target("test")
    set_default(true)
    set_kind("binary")
    add_tests("coal")

    add_packages("catch2")

    add_includedirs("include", "tests")
    add_files("tests/**.cpp")
