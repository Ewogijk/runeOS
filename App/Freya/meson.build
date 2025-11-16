project('Freya', 'cpp')
add_project_arguments('-mincoming-stack-boundary=3', language : 'c')
add_project_arguments('-mincoming-stack-boundary=3', language : 'cpp')

inc = [
    'include'
]

yaml_cpp_proj = subproject('yaml-cpp')
yaml_cpp_dep = yaml_cpp_proj.get_variable('yaml_cpp_dep')

sources = [
    'src/DependencyGraph.cpp',
    'src/ExitCode.cpp',
    'src/main.cpp',
    'src/Service.cpp',
    'src/ServiceLoader.cpp',
    'src/ServiceStarter.cpp',
]

executable('Freya.app',
           sources,
           cpp_args : '-std=c++20',
           dependencies : yaml_cpp_dep,
           include_directories : inc
)