#include "drcheck/io/TclAutomationRunner.h"

#include "drcheck/engine/DRCRunner.h"

#include <tcl.h>

#include <exception>
#include <map>
#include <stdexcept>
#include <string>

namespace drcheck::io {
namespace {

struct AutomationContext
{
    std::vector<domain::Violation> violations;
};

// drc_run -layout layoutPath -rules rulesPath -report reportPath [-svg svgPath] [-top topCellName]
int drcRunCommand(ClientData clientData, Tcl_Interp* interpreter, int objc, Tcl_Obj* const objv[])
{
    auto* context = static_cast<AutomationContext*>(clientData);

    try
    {
        if ((objc - 1) % 2 != 0)
        {
            throw std::invalid_argument("drc_run options must be provided as -option value pairs");
        }

        std::map<std::string, std::string> options;

        for (int i = 1; i < objc; i += 2)
        {
            const std::string option = Tcl_GetString(objv[i]);
            const std::string value = Tcl_GetString(objv[i + 1]);

            if (option.empty() || option[0] != '-')
            {
                throw std::invalid_argument("Invalid drc_run option: " + option);
            }

            if (options.contains(option))
            {
                throw std::invalid_argument("Duplicate drc_run option: " + option);
            }

            options.emplace(option, value);
        }

        if (!options.contains("-layout"))
        {
            throw std::invalid_argument("drc_run requires -layout");
        }

        if (!options.contains("-rules"))
        {
            throw std::invalid_argument("drc_run requires -rules");
        }

        if (!options.contains("-report"))
        {
            throw std::invalid_argument("drc_run requires -report");
        }

        for (const auto& [option, value] : options)
        {
            if (option != "-layout" && option != "-rules" && option != "-report" && option != "-svg" && option != "-top")
            {
                throw std::invalid_argument("Unknown drc_run option: " + option);
            }
        }

        engine::DRCRunConfig config;

        config.layoutPath = options.at("-layout");
        config.rulesPath = options.at("-rules");
        config.reportPath = options.at("-report");
        if (options.contains("-svg"))
        {
            config.svgPath = options.at("-svg");
        }
        if (options.contains("-top"))
        {
            config.topCellName = options.at("-top");
        }
        context->violations = engine::DRCRunner::run(config);

        return TCL_OK;
    }
    catch (const std::exception& exception)
    {
        Tcl_SetObjResult(interpreter, Tcl_NewStringObj(exception.what(), -1));
        return TCL_ERROR;
    }
}
//  drc_error_count
int drcErrorCountCommand(ClientData clientData, Tcl_Interp* interpreter, int objc, Tcl_Obj* const objv[])
{
    auto* context = static_cast<AutomationContext*>(clientData);

    if (objc != 1)
    {
        Tcl_WrongNumArgs(interpreter, 1, objv, "");
        return TCL_ERROR;
    }

    Tcl_SetObjResult(interpreter, Tcl_NewWideIntObj(static_cast<Tcl_WideInt>(context->violations.size())));

    return TCL_OK;
}
}
std::vector<domain::Violation> TclAutomationRunner::run(const std::string& filePath)
{
    Tcl_Interp* interpreter = Tcl_CreateInterp();

    if (interpreter == nullptr)
    {
        throw std::runtime_error("Failed to create Tcl interpreter");
    }

    AutomationContext context;

    Tcl_CreateObjCommand(interpreter, "drc_run", drcRunCommand, &context, nullptr);
    Tcl_CreateObjCommand(interpreter, "drc_error_count", drcErrorCountCommand, &context, nullptr);

    const int result = Tcl_EvalFile(interpreter, filePath.c_str());

    if (result != TCL_OK)
    {
        const std::string errorMessage = Tcl_GetString(Tcl_GetObjResult(interpreter));

        Tcl_DeleteInterp(interpreter);

        throw std::invalid_argument("Failed to execute Tcl automation script: " + errorMessage);
    }

    Tcl_DeleteInterp(interpreter);

    return context.violations;
}
}