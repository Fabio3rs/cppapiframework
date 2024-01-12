#include "projstdafx.hpp"

// NOLINTNEXTLINE(hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
auto apiframework::mainboot(int /* argc */, char * /* argv */[], char **envp) -> int {
    CConfig::config().load_from_envp(envp);

    return 0;
}
