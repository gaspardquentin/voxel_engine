#include "voxel_engine/command_registry.h"
#include <sstream>


void voxeng::CommandRegistry::registerCommand(const std::string& name, CommandHandler handler) {
    m_commands.insert({ name, handler });
}


bool voxeng::CommandRegistry::tryExecute(const std::string& input, CommandContext& ctx) {
    if (input.length() == 0) {
        return false;
    }

    std::vector<std::string> argv;
    std::stringstream ss(input);

    std::string tmp;
    while (getline(ss, tmp, ' ')) {
        argv.push_back(tmp);
    }

    if (argv[0][0] != COMMAND_PREFIX) {
        return false;
    }

    std::string command_name = argv[0];
    argv.erase(argv.begin());

    //TODO: maybe remove the '/' from the command name ? or register command names with their /
    if (m_commands.find(command_name) == m_commands.end()) {
        return false;
    }
    CommandHandler& command = m_commands.at(command_name);
    command(argv, ctx);

    return true;



}
