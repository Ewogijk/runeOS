project('Freya', 'cpp', default_options : ['cpp_std=c++20'])
add_project_arguments('-mincoming-stack-boundary=3', language : 'c')
add_project_arguments('-mincoming-stack-boundary=3', language : 'cpp')

inc = [
    'include'
]

yaml_cpp_proj = subproject('yaml-cpp')
yaml_cpp_dep = yaml_cpp_proj.get_variable('yaml_cpp_dep')

sources = [
    'src/Freya/DependencyGraph.cpp',
    'src/Freya/ExitCode.cpp',
    'src/main.cpp',
    'src/Freya/Service.cpp',
    'src/Freya/ServiceLoader.cpp',
    'src/Freya/ServiceStarter.cpp',
]

executable('Freya.app',
           sources,
           cpp_args : '-std=c++20',
           dependencies : yaml_cpp_dep,
           include_directories : inc
)