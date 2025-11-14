project('yaml-cpp', 'cpp')
add_project_arguments('-mincoming-stack-boundary=3', language : 'c')
add_project_arguments('-mincoming-stack-boundary=3', language : 'cpp')

inc_dir = include_directories(
    'include'
)
sources = [
    'src/binary.cpp',
    'src/binary.cpp',
    'src/binary.cpp',
    'src/binary.cpp',
    'src/node.cpp',
    'src/contrib/graphbuilder.cpp',
    'src/contrib/graphbuilderadapter.cpp',
    'src/parser.cpp',
    'src/emitterutils.cpp',
    'src/emitter.cpp',
    'src/nodebuilder.cpp',
    'src/tag.cpp',
    'src/node_data.cpp',
    'src/regex_yaml.cpp',
    'src/emitterstate.cpp',
    'src/depthguard.cpp',
    'src/null.cpp',
    'src/singledocparser.cpp',
    'src/directives.cpp',
    'src/exceptions.cpp',
    'src/scantoken.cpp',
    'src/emitfromevents.cpp',
    'src/scantag.cpp',
    'src/scanscalar.cpp',
    'src/nodeevents.cpp',
    'src/memory.cpp',
    'src/stream.cpp',
    'src/ostream_wrapper.cpp',
    'src/simplekey.cpp',
    'src/emit.cpp',
    'src/exp.cpp',
    'src/binary.cpp',
    'src/scanner.cpp',
    'src/parse.cpp',
    'src/convert.cpp',
]
yaml_cpp = static_library('yaml-cpp', sources, cpp_args : '-std=c++20', include_directories : inc_dir)
yaml_cpp_dep = declare_dependency(include_directories : inc_dir, link_with : yaml_cpp)