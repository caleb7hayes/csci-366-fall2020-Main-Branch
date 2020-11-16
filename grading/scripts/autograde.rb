#!/home/carson/.rvm/rubies/ruby-2.6.2/bin/ruby -w

require 'octokit'
require 'csv'

# =====================================================================
#  github setup
# =====================================================================
git_token = ENV["GITHUB_TOKEN"]
unless git_token
    throw "You must set the GITHUB_TOKEN environment variable"
end
client = Octokit::Client.new(:access_token => git_token)
client.auto_paginate = true
user = client.user
git_username= user.login
puts("github user id: #{user.login}")

# =====================================================================
#  helper functions
# =====================================================================
def each_student_dir(path)
    Dir.foreach(path) do |filename|
      next unless filename.end_with? "txt"
      dir_name = filename.split(/[.,]/)[0]
      destination = "#{path}/student_repos/#{dir_name}"
      if File.directory? destination
        yield destination
      else
        puts "Directory #{destination} does not exist, can't test"
      end
    end
end

def each_student
    results = CSV.read("students.csv", headers:true)
    results.each do |row|
        yield row
    end
end

def student_dir(student)
  student["FIRST_NAME"].downcase + "_" + student["LAST_NAME"].downcase
end


# =====================================================================
#  command line
# =====================================================================

command = ARGV[0]

if command == "accept_invites"
    puts("Accepting Repository Invites")
    puts("------------------")
    client.user_repository_invitations.each do |ri|
      puts "  Accepting #{ri[:id]}: #{ri[:repository][:full_name]}"
      client.accept_repo_invitation(ri[:id])
    end
elsif command == "grade"
    assignment = ARGV[1]
    if  assignment == "project"
        each_student do |student|
            dir = "repos/#{student_dir(student)}"
            puts "Grading #{student['FIRST_NAME']} #{student['LAST_NAME']} in #{dir}"
            puts
            `mkdir -p #{dir}/grading/
             cd #{dir}/battlebit;
             cmake .;
             make > tmp.out 2> err.out;
             timeout 3 ./googletest/battleBit_tests > ../grading/project_results.txt;
             git add ../grading/*;
             git commit -m "From Autograder"
             git push`
        end
    else
        puts "Unknown assignment: #{assignment}"
    end
elsif command == "clone_repos"
    each_student do |student|
        repo_url = student["REPO"]
        `git clone https://#{git_username}:#{git_token}@#{repo_url} repos/#{student_dir(student)}`
    end
elsif command == "clear_repos"
    `rm -rf repos`
else
    puts "Commands:
    accept_invites - accepts pending invites
    clone_repos - clones student repos to the repos directory
    clear_repos - removes all repos from the current dir
    grade <assignment> - grades the given assignment and pushes it"
end
