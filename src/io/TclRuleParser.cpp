#include "drcheck/io/TclRuleParser.h"

#include "drcheck/domain/Layer.h"
#include "drcheck/rules/RuleFactory.h"
#include "drcheck/geometry/BoundingBox.h"
#include "drcheck/rules/MinEnclosureRule.h"

#include <tcl.h>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>

namespace drcheck::io {
namespace {
// Make options and their value in a map to enable tcl command without arranging options 
std::map<std::string, Tcl_Obj*> parseOptions(int objc, Tcl_Obj* const objv[])
{
    // Validate options
    if (objc < 2)
    {
        throw std::invalid_argument("Missing rule type");
    }

    if ((objc - 2) % 2 != 0)
    {
        throw std::invalid_argument("Rule options must be provided as -option value pairs");
    }
    // map between each option and it's value
    std::map<std::string, Tcl_Obj*> options;

    for (int i = 2; i < objc; i += 2)
    {
        const std::string option = Tcl_GetString(objv[i]);

        if (option.empty() || option[0] != '-')
        {
            throw std::invalid_argument("Invalid rule option: " + option);
        }

        if (options.contains(option))
        {
            throw std::invalid_argument("Duplicate rule option: " + option);
        }

        options.emplace(option, objv[i + 1]);
    }

    return options;
}

double getDoubleOption(Tcl_Interp* interpreter, Tcl_Obj* optionObj) {
    double value = 0.0;
    if (Tcl_GetDoubleFromObj(interpreter, optionObj, &value) != TCL_OK)
    {
        throw std::invalid_argument(Tcl_GetString(Tcl_GetObjResult(interpreter)));
    }
    return value;
}

rules::DensityLimit parseDensityLimit(const std::string& limit)
{
    if (limit == "minimum")
    {
        return rules::DensityLimit::Minimum;
    }

    if (limit == "maximum")
    {
        return rules::DensityLimit::Maximum;
    }

    throw std::invalid_argument("Invalid density limit: " + limit);
}
geometry::BoundingBox parseRegion(Tcl_Interp* interpreter, Tcl_Obj* regionObj)
{
    #if TCL_MAJOR_VERSION >= 9
    Tcl_Size elementCount = 0;
    #else
    int elementCount = 0;
    #endif
    Tcl_Obj** elements = nullptr;

    if (Tcl_ListObjGetElements(interpreter, regionObj, &elementCount, &elements) != TCL_OK)
    {
        throw std::invalid_argument(Tcl_GetString(Tcl_GetObjResult(interpreter)));
    }

    if (elementCount != 4)
    {
        throw std::invalid_argument("Density region requires four values: minX minY maxX maxY");
    }

    const double minX = getDoubleOption(interpreter, elements[0]);
    const double minY = getDoubleOption(interpreter, elements[1]);
    const double maxX = getDoubleOption(interpreter, elements[2]);
    const double maxY = getDoubleOption(interpreter, elements[3]);

    return geometry::BoundingBox(minX, minY, maxX, maxY);
}

std::vector<rules::EnclosureOption> parseEnclosureOptions(Tcl_Interp* interpreter, Tcl_Obj* optionsObj)
{
    #if TCL_MAJOR_VERSION >= 9
    Tcl_Size optionCount = 0;
    #else
    int optionCount = 0;
    #endif

    Tcl_Obj** optionObjects = nullptr;

    if (Tcl_ListObjGetElements(interpreter, optionsObj, &optionCount, &optionObjects) != TCL_OK)
    {
        throw std::invalid_argument(Tcl_GetString(Tcl_GetObjResult(interpreter)));
    }

    if (optionCount == 0)
    {
        throw std::invalid_argument("min_enclosure -options must contain at least one enclosure option");
    }

    std::vector<rules::EnclosureOption> enclosureOptions;
    enclosureOptions.reserve(optionCount);

    for (int i = 0; i < optionCount; ++i)
    {
        #if TCL_MAJOR_VERSION >= 9
        Tcl_Size elementCount = 0;
        #else
        int elementCount = 0;
        #endif

        Tcl_Obj** elements = nullptr;

        if (Tcl_ListObjGetElements(interpreter, optionObjects[i], &elementCount, &elements) != TCL_OK)
        {
            throw std::invalid_argument(Tcl_GetString(Tcl_GetObjResult(interpreter)));
        }

        if (elementCount % 2 != 0)
        {
            throw std::invalid_argument("Enclosure option values must be provided as -option value pairs");
        }

        std::map<std::string, Tcl_Obj*> optionValues;

        for (int j = 0; j < elementCount; j += 2)
        {
            const std::string option = Tcl_GetString(elements[j]);

            if (option.empty() || option[0] != '-')
            {
                throw std::invalid_argument("Invalid enclosure option: " + option);
            }

            if (optionValues.contains(option))
            {
                throw std::invalid_argument("Duplicate enclosure option: " + option);
            }

            optionValues.emplace(option, elements[j + 1]);
        }

        if (!optionValues.contains("-outer") || !optionValues.contains("-all_sides"))
        {
            throw std::invalid_argument("Enclosure option requires -outer and -all_sides");
        }

        const bool hasFirstPair = optionValues.contains("-first_pair");
        const bool hasSecondPair = optionValues.contains("-second_pair");

        if (hasFirstPair != hasSecondPair)
        {
            throw std::invalid_argument("Enclosure option must provide both -first_pair and -second_pair");
        }

        if ((!hasFirstPair && optionValues.size() != 2) || (hasFirstPair && optionValues.size() != 4))
        {
            throw std::invalid_argument("Unknown min_enclosure option");
        }

        const domain::Layer outerLayer = domain::layerFromString(Tcl_GetString(optionValues.at("-outer")));

        const double allSidesMinEnclosure = getDoubleOption(interpreter, optionValues.at("-all_sides"));

        if (!hasFirstPair)
        {
            enclosureOptions.emplace_back(outerLayer, allSidesMinEnclosure);
            continue;
        }

        const double firstPairMinEnclosure = getDoubleOption(interpreter, optionValues.at("-first_pair"));
        const double secondPairMinEnclosure = getDoubleOption(interpreter, optionValues.at("-second_pair"));

        enclosureOptions.emplace_back(outerLayer, allSidesMinEnclosure, firstPairMinEnclosure, secondPairMinEnclosure);
    }
    return enclosureOptions;
}

int ruleCommand(ClientData clientData, Tcl_Interp* interpreter, int objc, Tcl_Obj* const objv[])
{
    auto* rules = static_cast<std::vector<std::unique_ptr<rules::Rule>>*>(clientData);

    try
    {
        if (objc < 2)
        {
            throw std::invalid_argument("Missing rule type");
        }

        const std::string type = Tcl_GetString(objv[1]);
        const auto options = parseOptions(objc, objv);

        rules::RuleParameters params;

        if (type == "min_spacing" || type == "min_width")
        {
            if (options.size() != 2 || !options.contains("-layer") || !options.contains("-value"))
            {
                throw std::invalid_argument(type + " requires -layer and -value");
            }
            params.layer = domain::layerFromString(Tcl_GetString(options.at("-layer")));
            params.value = getDoubleOption(interpreter, options.at("-value"));
        }
        else if (type == "min_enclosure")
        {
            if (!options.contains("-inner"))
            {
                throw std::invalid_argument("min_enclosure requires -inner");
            }

            params.innerLayer = domain::layerFromString(Tcl_GetString(options.at("-inner")));

            if (options.contains("-options"))
            {
                if (options.size() != 2)
                {
                    throw std::invalid_argument("min_enclosure with -options requires only -inner and -options");
                }

                params.enclosureOptions = parseEnclosureOptions(interpreter, options.at("-options"));
            }
            else
            {
                if (options.size() != 3 || !options.contains("-outer") || !options.contains("-value"))
                {
                    throw std::invalid_argument("min_enclosure requires either -inner, -outer and -value or -inner and -options");
                }

                params.outerLayer = domain::layerFromString(Tcl_GetString(options.at("-outer")));
                params.value = getDoubleOption(interpreter, options.at("-value"));
            }
        }
        else if (type == "density")
        {
            if ((options.size() != 5 && options.size() != 6) ||!options.contains("-layer") || !options.contains("-limit") || !options.contains("-value") || !options.contains("-window_size") || !options.contains("-window_step"))
            {
                throw std::invalid_argument("density requires -layer, -limit, -value, -window_size and -window_step");
            }
            if (options.size() == 6 && !options.contains("-region"))
            {
                throw std::invalid_argument("Unknown density option");
            }
            params.layer = domain::layerFromString(Tcl_GetString(options.at("-layer")));
            params.densityLimit = parseDensityLimit(Tcl_GetString(options.at("-limit")));
            params.value = getDoubleOption(interpreter, options.at("-value"));
            params.windowSize = getDoubleOption(interpreter, options.at("-window_size"));
            params.windowStep = getDoubleOption(interpreter, options.at("-window_step"));

            if (options.contains("-region"))
            {
                params.analysisWindow = parseRegion(interpreter, options.at("-region"));
            }
        }
        else
        {
            throw std::invalid_argument("Unsupported rule type: " + type);
        }

        rules->push_back(rules::RuleFactory::create(type, params));
    }
    catch (const std::exception& exception)
    {
        Tcl_SetObjResult(interpreter, Tcl_NewStringObj(exception.what(), -1));
        return TCL_ERROR;
    }

    return TCL_OK;
}
}

std::vector<std::unique_ptr<rules::Rule>> TclRuleParser::load(const std::string& filePath)
{
    Tcl_Interp* interpreter = Tcl_CreateInterp();

    if (interpreter == nullptr)
    {
        throw std::runtime_error("Failed to create Tcl interpreter");
    }

    std::vector<std::unique_ptr<rules::Rule>> rules;
    // Tcl_CreateObjCommand
    // (interpreter, Tcl command name, C++ callback function, custom C++ data pointer passed to callback, cleanup callback);
    // Pointer: the callback function will store command args in the pointer to be used after callback ends, otherwise args will be removed after callback ends
    Tcl_CreateObjCommand(interpreter, "rule", ruleCommand, &rules, nullptr);

    const int result = Tcl_EvalFile(interpreter, filePath.c_str());

    if (result != TCL_OK)
    {
        const std::string errorMessage = Tcl_GetString(Tcl_GetObjResult(interpreter));

        Tcl_DeleteInterp(interpreter);

        throw std::invalid_argument("Failed to parse Tcl rule file: " + errorMessage);
    }

    Tcl_DeleteInterp(interpreter);

    return rules;
}
}