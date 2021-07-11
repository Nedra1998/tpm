#ifndef EXIT_CODE_HPP_W0ZGXHBD
#define EXIT_CODE_HPP_W0ZGXHBD

namespace tpm
{
  enum ExitCode {
    OK = 0,
    EXIT_OK = 1,
    LOG_INIT,

    ARGPARSE_MISSING_POSITIONAL,

    SCENE_NOT_FOUND,
    SCENE_PARSE_ERROR,
    SCENE_MISSING,


    MKDIR_ERROR,
    IMG_WRITE_ERR
};
} /* tpm */ 

#endif /* end of include guard: EXIT_CODE_HPP_W0ZGXHBD */
