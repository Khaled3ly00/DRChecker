#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

#include "drcheck/engine/DRCRunner.h"
#include "drcheck/io/TclAutomationRunner.h"

int main(int argc, char* argv[])
{
    // Usage: drcheck --layout layout.json --rules rules.tcl --report report.json [--svg report.svg]
    try
    {
        if (argc >= 2 && std::string(argv[1]) == "--script" && argc != 3)
        {
            throw std::invalid_argument("Usage: drcheck --script <automation.tcl>");
        }
        if (argc == 3 && std::string(argv[1]) == "--script")
        {
            const std::string scriptPath = argv[2];

            const auto violations = drcheck::io::TclAutomationRunner::run(scriptPath);

            std::cout
                << "Script completed.\n";

            return 0;
        }
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

        drcheck::engine::DRCRunConfig config;

        config.layoutPath = layoutPath;
        config.rulesPath = rulesPath;
        config.reportPath = reportPath;
        if (!svgPath.empty())
        {
            config.svgPath = svgPath;
        }

        const auto violations = drcheck::engine::DRCRunner::run(config);

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