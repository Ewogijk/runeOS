project('runeOS', 'cpp')

# Generate the build info passed to the OS
conf_data = configuration_data()
conf_data.set('MAJOR', get_option('major'))
conf_data.set('MINOR', get_option('minor'))
conf_data.set('PATCH', get_option('patch'))
conf_data.set('PRERELEASE', '"' + get_option('prerelease')  + '"')
configure_file(
        input: 'Build.h.in',
        output: 'Build.h',
        configuration: conf_data
)


includes = include_directories('Include')
sources = files(
    'Src/Shell/Action.cpp',
    'Src/Shell/AST.cpp',
    'Src/Shell/AutoCompletion.cpp',
    'Src/Shell/BuiltInCommand.cpp',
    'Src/Shell/Environment.cpp',
    'Src/Shell/Interpreter.cpp',
    'Src/Shell/Lexer.cpp',
    'Src/Shell/Parser.cpp',
    'Src/Shell/Path.cpp',
    'Src/Shell/Utility.cpp',
    'Src/OS.cpp',
)

executable('runeOS.app', sources, include_directories : includes)