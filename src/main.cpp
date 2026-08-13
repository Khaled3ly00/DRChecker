#include <iostream>

#include "drcheck/engine/DRCEngine.h"
#include "drcheck/io/JSONLayoutParser.h"
#include "drcheck/io/JSONRuleParser.h"

int main(int argc, char* argv[])
{
    // Required argument count = 3 
    // argv[0] → executable name 
    // argv[1] → layout path 
    // argv[2] → rules path
    if (argc != 3) {
        std::cerr<< "Usage: drchecker <layout.json> <rules.json>\n";
        return 1;
    }

    try
    {
        drcheck::io::JSONLayoutParser layoutParser;
        drcheck::io::JSONRuleParser ruleParser;
        drcheck::engine::DRCEngine engine;

        const auto shapes = layoutParser.load(argv[1]);

        const auto rules = ruleParser.load(argv[2]);

        const auto violations = engine.run(shapes, rules);

        std::size_t index = 1;

        for (const auto& violation : violations)
        {
            std::cout
                << "[" << index++ << "] "
                << violation.getTypeAsString()
                << '\n';

            std::cout
                << "  Message: "
                << violation.getMessage()
                << '\n';

            std::cout
                << "  Shapes: ";

            for (const std::size_t id :violation.getShapeIds())
            {
                std::cout << id << ' ';
            }
            std::cout << "\n\n";
        }

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