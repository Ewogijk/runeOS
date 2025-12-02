project('Rune Integration Test', 'cpp')
add_project_arguments('-mincoming-stack-boundary=3', language : 'c')
add_project_arguments('-mincoming-stack-boundary=3', language : 'cpp')

inc = [
    '../../Include'
]

sources = [
    'Heimdall/AssertionHandler.cpp',
    'Heimdall/Engine.cpp',
    'Heimdall/TestTracker.cpp',
    'IntegrationTest/hre/HRE.cpp',
    'IntegrationTest/hre/HString.cpp',
    'IntegrationTest/hre/ReporterRegistry.cpp',
    'IntegrationTest/hre/StdReporter.cpp',
    'IntegrationTest/hre/TestList.cpp',
    'IntegrationTest/hre/TestTracker.cpp',
    'IntegrationTest/main.cpp',
]

executable('rune_integration_test.app',
           sources,
           cpp_args : '-std=c++20',
           include_directories : inc)