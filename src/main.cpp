#include <iostream>

#include "drcheck/engine/DRCEngine.h"
#include "drcheck/io/JSONLayoutParser.h"
#include "drcheck/io/JSONRuleParser.h"
#include "drcheck/io/JSONReportWriter.h"
#include "drcheck/io/SVGReportWriter.h"

int main(int argc, char* argv[])
{
    // Required argument count = 3 
    // argv[0] → executable name 
    // argv[1] → layout path 
    // argv[2] → rules path
    // argv[3] → JSON output report path
    // argv[4] → SVG output report path
    if (argc != 4 && argc != 5) {
        std::cerr<< "Usage: drchecker <layout.json> <rules.json> <report.json>  [report.svg]\n";
        return 1;
    }

    try
    {
        drcheck::io::JSONLayoutParser layoutParser;
        drcheck::io::JSONRuleParser ruleParser;
        drcheck::engine::DRCEngine engine;
        drcheck::io::JSONReportWriter JSONWriter;

        const auto shapes = layoutParser.load(argv[1]);

        const auto rules = ruleParser.load(argv[2]);

        const auto violations = engine.run(shapes, rules);

        JSONWriter.write(violations, argv[3]);

        const bool writeSvg = argc == 5;

        if (argc == 5)
        {
            const std::string svgPath = argv[4];
            drcheck::io::SVGReportWriter::write(shapes, violations, svgPath);
        }

        std::cout
            << "DRC completed.\n"
            << "Violations: "
            << violations.size()
            << '\n';

        return 0;
    }
    catch (const std::exception& exception)
    {
        std::cerr
            << "Error: "
            << exception.what()
            << '\n';

        return 1;
    }
}