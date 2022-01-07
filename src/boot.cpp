#include "boot.hpp"
#include "utils/CConfig.hpp"

auto apiframework::mainboot(int /* argc */, char * /* argv */[], char **envp) -> int {
    CConfig::config().load_from_envp(envp);

    return 0;
}
