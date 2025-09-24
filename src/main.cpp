#ifdef __PSP__
#include <exception>
#include <pspdebug.h>
#include <pspdisplay.h>

[[noreturn]] void terminate() noexcept {
  pspDebugScreenInit();
  pspDebugScreenSetXY(0, 0);

  if (auto ex = std::current_exception()) {
    try {
      std::rethrow_exception(ex);
    } catch (const std::exception &e) {
      pspDebugScreenPrintf("%s\n", e.what());
    } catch (...) {
      pspDebugScreenPrintf("Unknown exception\n");
    }
  } else {
    pspDebugScreenPrintf("Terminate called without exception\n");
  }

  while (1)
    sceDisplayWaitVblankStart();
}
#endif

int main(int argc, char *argv[]) {
#ifdef __PSP__
  std::set_terminate(terminate);
#endif

  return 0;
}
