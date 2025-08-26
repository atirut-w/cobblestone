#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspkernel.h>

PSP_MODULE_INFO("Cobblestone", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);

int main() {
  pspDebugScreenInit();
  while (1) {
    pspDebugScreenSetXY(0, 0);
    pspDebugScreenPrintf("Hello, world!");
    sceDisplayWaitVblank();
  }

  return 0;
}
