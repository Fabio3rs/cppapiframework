#include "boot.hpp"
#include "utils/CConfig.hpp"

int apiframework::mainboot(int argc, char *argv[], char **envp) {
    std::cout << "argn " << argc << std::endl;
    std::cout << "name " << argv[0] << std::endl;

    CConfig::config().load_from_envp(envp);

    return 0;
}
