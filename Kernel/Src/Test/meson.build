project('Rune Integration Test', 'cpp')
add_project_arguments('-mincoming-stack-boundary=3', language : 'c')
add_project_arguments('-mincoming-stack-boundary=3', language : 'cpp')

inc = [
    '../../Include'
]

sources = [
    'Heimdall/AssertionHandler.cpp',
    'Heimdall/ConsoleReporter.cpp',
    'Heimdall/Engine.cpp',
    'Heimdall/GnomeReporter.cpp',
    'Heimdall/JUnitReporter.cpp',
    'Heimdall/TestTracker.cpp',
    'IntegrationTest/hre/HRE.cpp',
    'IntegrationTest/hre/HString.cpp',
    'IntegrationTest/hre/JUnitTestList.cpp',
    'IntegrationTest/hre/JUnitTestSuiteList.cpp',
    'IntegrationTest/hre/OptionList.cpp',
    'IntegrationTest/hre/ReporterRegistry.cpp',
    'IntegrationTest/hre/TestList.cpp',
    'IntegrationTest/hre/TestTracker.cpp',
    'IntegrationTest/main.cpp',
]

executable('IntegrationTest.app',
           sources,
           cpp_args : '-std=c++20',
           include_directories : inc)