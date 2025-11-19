project('Crucible', 'cpp')

includes = include_directories('Include')
sources = files(
    'Src/Crucible/Action.cpp',
    'Src/Crucible/AST.cpp',
    'Src/Crucible/AutoCompletion.cpp',
    'Src/Crucible/BuiltInCommand.cpp',
    'Src/Crucible/Environment.cpp',
    'Src/Crucible/Interpreter.cpp',
    'Src/Crucible/Lexer.cpp',
    'Src/Crucible/Parser.cpp',
    'Src/Crucible/Path.cpp',
    'Src/Crucible/Utility.cpp',
    'Src/Crucible.cpp',
)

add_project_arguments('-mincoming-stack-boundary=3', language : 'c')
add_project_arguments('-mincoming-stack-boundary=3', language : 'cpp')

executable('Crucible.app', sources, include_directories : includes)