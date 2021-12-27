#include "boot.hpp"
#include "utils/CConfig.hpp"

int apiframework::mainboot(int /* argc */, char */* argv */[], char **envp) {
    CConfig::config().load_from_envp(envp);

    return 0;
}
