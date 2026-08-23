#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

#include "drcheck/engine/DRCEngine.h"
#include "drcheck/io/JSONLayoutParser.h"
#include "drcheck/io/JSONRuleParser.h"
#include "drcheck/io/JSONReportWriter.h"
#include "drcheck/io/SVGReportWriter.h"
#include "drcheck/io/TclRuleParser.h"

int main(int argc, char* argv[])
{
    // Usage: drcheck --layout layout.json --rules rules.tcl --report report.json [--svg report.svg]
    try
    {
        std::string layoutPath;
        std::string rulesPath;
        std::string reportPath;
        std::string svgPath;

        for (int i = 1; i < argc; i += 2)
        {
            if (i + 1 >= argc)
            {
                throw std::invalid_argument("Missing value for argument: " + std::string(argv[i]));
            }

            const std::string argument = argv[i];
            const std::string value = argv[i + 1];

            if (argument == "--layout")
            {
                layoutPath = value;
            }
            else if (argument == "--rules")
            {
                rulesPath = value;
            }
            else if (argument == "--report")
            {
                reportPath = value;
            }
            else if (argument == "--svg")
            {
                svgPath = value;
            }
            else
            {
                throw std::invalid_argument("Usage: drcheck --layout layout.json --rules rules.tcl --report report.json [--svg report.svg]");
            }
        }

        if (layoutPath.empty())
        {
            throw std::invalid_argument("Missing required argument: --layout");
        }

        if (rulesPath.empty())
        {
            throw std::invalid_argument("Missing required argument: --rules");
        }

        if (reportPath.empty())
        {
            throw std::invalid_argument("Missing required argument: --report");
        }

        const auto shapes = drcheck::io::JSONLayoutParser::load(layoutPath);

        std::vector<std::unique_ptr<drcheck::rules::Rule>> rules;

        const std::string extension = std::filesystem::path(rulesPath).extension().string();

        if (extension == ".json")
        {
            rules = drcheck::io::JSONRuleParser::load(rulesPath);
        }
        else if (extension == ".tcl")
        {
            rules = drcheck::io::TclRuleParser::load(rulesPath);
        }
        else
        {
            throw std::invalid_argument("Unsupported rule file format: " + extension);
        }

        const auto violations = drcheck::engine::DRCEngine::run(shapes, rules);

        drcheck::io::JSONReportWriter::write(violations, reportPath);

        if (!svgPath.empty())
        {
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