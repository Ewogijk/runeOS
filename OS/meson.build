project('runeOS', 'cpp')

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