#!/usr/bin/ruby -Ku

require "csv"

OUTPUT_DIR = "cases"

def main
  lognames = Dir.glob(OUTPUT_DIR + "/link*.log")
  CSV.open("result.csv", "wb") do |csv|
    csv << ["bandwidth", "latency", "core", "stime", "atime"]
    lognames.each do |logname|
      s = File.basename(logname, ".*").split("_")
      band = s[1]
      latency = s[2]
      core = /core([0-9]+)/.match(s[3])[1]
      stime = 0
      atime = 0
      File.open(logname, "r") do |f|
        f.each_line do |line|
          if line.include?("Simulated time:")
            stime = /(\S+) seconds/.match(line)[1]
          end
          if line.include?("seconds were actual computation of the application")
            atime = /(\S+) seconds/.match(line)[1]
          end
        end
      end
      csv << [band, latency, core, stime, atime]
    end
  end
end

main if $0 == __FILE__

