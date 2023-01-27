require 'rake/testtask'

Rake::TestTask.new do |t|
  t.libs << 'test'
end

desc "Run tests"
task :default => :test

desc "build and install locally"
task :install do
  puts "uninstalling"
  `gem uninstall swe4r`
  puts "building"
  `gem build`
  `gem install --local ./swe4r-1.0.gem`
  puts "done!"
end
