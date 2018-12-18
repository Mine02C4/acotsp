#!/usr/bin/ruby -Ku

require 'rexml/document'

OUTPUT_DIR = "cases"
BASE_XML = "cases/base.xml"

LINK_BANDWIDTH_LIST = ["1.0e+16", "1.0e+9", "1.0e+10"]
LINK_LATENCY_LIST = ["2.0e-07", "0.001", "0.1"]

def main
  # Prepare Output directory
  if !File::directory?(OUTPUT_DIR)
    Dir::mkdir(OUTPUT_DIR)
  end
  if File.exist?(BASE_XML) and File::ftype(BASE_XML) == "file"
    File.open(BASE_XML) {|fp|
      doc = REXML::Document.new fp
      LINK_BANDWIDTH_LIST.each do |band|
        LINK_LATENCY_LIST.each do |latency|
          fname = "link_%s_%s.xml" % [band, latency]
          doc.elements.each("platform/AS/link") do |elem|
            elem.attributes["bandwidth"] = band
            elem.attributes["latency"] = latency
          end
          File.open(OUTPUT_DIR + "/" + fname, "w") do |outfile|
            doc.write(outfile, 0)
          end
        end
      end
    }
  else
    puts "XML Error: \"" + BASE_XML + "\" is not a file."
    exit
  end
end

main if $0 == __FILE__

