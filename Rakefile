#
#
#

%w( colorize rake fileutils ).each do |gem|
  begin
    require gem
  rescue LoadError
    warn "Install the #{gem} gem:\n $ (sudo) gem install #{gem}".magenta
    exit 1
  end
end

require "rake/clean"
require_relative "./Rakefile_common.rb"

CLEAN.include   ["./**/*.o", "./**/*.obj", "./bin/**/example*", "./build"]
CLOBBER.include []
CLEAN.exclude('**/[cC][oO][rR][eE]')

desc "run tests"
task :run do
  sh "./bin/test1"
  sh "./bin/test2"
  sh "./bin/test3"
  sh "./bin/test4"
  sh "./bin/test5"
  sh "./bin/test6"
  ##sh "./bin/test7"
  sh "./bin/test8"
  sh "./bin/test9"
end

desc "run tests"
task :run_win do
  sh "./bin/Release/test1"
  sh "./bin/Release/test2"
  sh "./bin/Release/test3"
  sh "./bin/Release/test4"
  sh "./bin/Release/test5"
  sh "./bin/Release/test6"
  ##sh "./bin/Release/test7"
  sh "./bin/Release/test8"
  sh "./bin/Release/test9"
end

desc "compile for Visual Studio [default year=2017, bits=x64]"
task :build_win, [:year, :bits] do |t, args|

  args.with_defaults( :year => "2017", :bits => "x64" )

  Rake::Task[:win_3rd].invoke(args.year,args.bits,args.lapack)

  dir = "vs_#{args.year}_#{args.bits}"

  FileUtils.rm_rf   dir
  FileUtils.mkdir_p dir
  FileUtils.cd      dir

  cmake_cmd = win_vs(args.bits,args.year)
  cmake_cmd += " -DGC_DIR:VAR=#{args.gc_dir} "
  if COMPILE_EXECUTABLE then
    cmake_cmd += ' -DBUILD_EXECUTABLE:VAR=true '
  else
    cmake_cmd += ' -DBUILD_EXECUTABLE:VAR=false '
  end
  if COMPILE_DYNAMIC then
    cmake_cmd += ' -DBUILD_SHARED:VAR=true '
  else
    cmake_cmd += ' -DBUILD_SHARED:VAR=false '
  end

  FileUtils.mkdir_p "../lib/lib"
  FileUtils.mkdir_p "../lib/bin"
  FileUtils.mkdir_p "../lib/bin/"+args.bits
  FileUtils.mkdir_p "../lib/dll"
  FileUtils.mkdir_p "../lib/include"

  if COMPILE_DEBUG then
    sh cmake_cmd + ' -DCMAKE_BUILD_TYPE:VAR=Debug --loglevel=WARNING ..'
    sh 'cmake --build . --config Debug --target install '+PARALLEL+QUIET
  else
    sh cmake_cmd + ' -DCMAKE_BUILD_TYPE:VAR=Release --loglevel=WARNING ..'
    sh 'cmake  --build . --config Release --target install '+PARALLEL+QUIET
  end
  FileUtils.cd '..'
end


desc "compile for OSX"
task :build, [:os] do |t, args|

  args.with_defaults( :os => "osx" )

  case :os
  when 'osx'
    Rake::Task[:osx_3rd].invoke()
  when 'linux'
    Rake::Task[:linux_3rd].invoke()
  end

  dir = "build"

  FileUtils.rm_rf   dir
  FileUtils.mkdir_p dir
  FileUtils.cd      dir

  cmake_cmd = "cmake "

  if COMPILE_EXECUTABLE then
    cmake_cmd += '-DBUILD_EXECUTABLE:VAR=true '
  else
    cmake_cmd += '-DBUILD_EXECUTABLE:VAR=false '
  end
  if COMPILE_DYNAMIC then
    cmake_cmd += '-DBUILD_SHARED:VAR=true '
  else
    cmake_cmd += '-DBUILD_SHARED:VAR=false '
  end

  if COMPILE_DEBUG then
    sh cmake_cmd + '-DCMAKE_BUILD_TYPE:VAR=Debug .. ' #--loglevel=WARNING ..'
    sh 'cmake --build . --config Debug --target install '+PARALLEL+QUIET
  else
    sh cmake_cmd + '-DCMAKE_BUILD_TYPE:VAR=Release .. ' #--loglevel=WARNING ..'
    sh 'cmake --build . --config Release --target install '+PARALLEL+QUIET
  end

  FileUtils.cd '..'
end

desc "compile for LINUX"
task :build_linux do
  Rake::Task[:build].invoke("linux")
end

desc "compile for OSX"
task :build_osx do
  Rake::Task[:build].invoke("osx")
end

desc 'install third parties for osx'
task :osx_3rd do
  FileUtils.rm_rf 'lib'
  FileUtils.rm_rf 'lib3rd'
  FileUtils.cp 'CMakeLists-cflags.txt', 'submodules/Utils/CMakeLists-cflags.txt'
  FileUtils.cp 'CMakeLists-cflags.txt', 'submodules/quarticRootsFlocke/CMakeLists-cflags.txt'
  FileUtils.cp 'CMakeLists-cflags.txt', 'submodules/GenericContainer/CMakeLists-cflags.txt'
  FileUtils.cd 'submodules'
  puts "\n\nSUBMODULES (for SPLINES)\n\n".green
  sh "rake build_osx"
  FileUtils.cd '..'
end

desc 'install third parties for linux'
task :linux_3rd do
  FileUtils.rm_rf 'lib'
  FileUtils.rm_rf 'lib3rd'
  FileUtils.cp 'CMakeLists-cflags.txt', 'submodules/Utils/CMakeLists-cflags.txt'
  FileUtils.cp 'CMakeLists-cflags.txt', 'submodules/quarticRootsFlocke/CMakeLists-cflags.txt'
  FileUtils.cp 'CMakeLists-cflags.txt', 'submodules/GenericContainer/CMakeLists-cflags.txt'
  FileUtils.cd 'submodules'
  puts "\n\nSUBMODULES (for SPLINES)\n\n".green
  sh "rake build_linux"
  FileUtils.cd '..'
end

desc "compile for Visual Studio [default year=2017, bits=x64]"
task :win_3rd, [:year, :bits] do |t, args|
  FileUtils.rm_rf 'lib'
  FileUtils.rm_rf 'lib3rd'
  FileUtils.cp 'CMakeLists-cflags.txt', 'submodules/Utils/CMakeLists-cflags.txt'
  FileUtils.cp 'CMakeLists-cflags.txt', 'submodules/quarticRootsFlocke/CMakeLists-cflags.txt'
  FileUtils.cp 'CMakeLists-cflags.txt', 'submodules/GenericContainer/CMakeLists-cflags.txt'
  args.with_defaults( :year => "2017", :bits => "x64" )
  FileUtils.cd 'submodules'
  puts "\n\nSUBMODULES (for SPLINES)\n\n".green
  sh "rake build_win[#{args.year},#{args.bits}]"
  FileUtils.cd '..'
end
